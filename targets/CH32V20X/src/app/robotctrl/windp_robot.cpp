#include <atomic>
#include <array>

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/string_view.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/timer/bipolarity_abstract.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio.hpp"

#include "types/regions/range2.hpp"
#include "types/vectors/vector2.hpp"
#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

#include "digipw/prelude/abdq.hpp"

using namespace ymd;

namespace PhysicalConstants {
    static constexpr iq16 L_s = 0.098_iq16;
    static constexpr iq16 L = 0.679_iq16;
    static constexpr iq16 G = 9.8_iq16;
    static constexpr iq16 h = 0.038_iq16;
    static constexpr iq16 H = L + h;
    static constexpr iq16 W = L + h;
    static constexpr iq16 M1 = 0.07_iq16;
    static constexpr iq16 M2 = 0.05_iq16;
    static constexpr iq16 M = M1+M2;
    static constexpr iq16 x = (M1 + 2*M2)/(2*M1 + 2*M2)*L;
    static constexpr iq16 J = M1*L*L/3 + M2*L*L;
}

static constexpr iq16 cali_iq16(const iq16 _x){
    iq16 x = CLAMP2(_x, 1.0_iq16);
    return sign(x)*(pow(abs(x), 1.5_iq16)+0.022_iq16);
}


class JetPwm{
public:
    JetPwm(
        hal::TimerOC & pwm_ap,
        hal::TimerOC & pwm_an,
        hal::TimerOC & pwm_bp,
        hal::TimerOC & pwm_bn
    ):
        channel_a_(pwm_ap, pwm_an),
        channel_b_(pwm_bp, pwm_bn)
    {;}

    void init_channels(){
        channel_a_.inverse(EN);
        channel_b_.inverse(DISEN);

        static constexpr hal::TimerOcPwmConfig pwm_noinv_cfg = {
            .cvr_sync_en = EN,
            .valid_level = HIGH
        };

        static constexpr hal::TimerOcPwmConfig pwm_inv_cfg = {
            .cvr_sync_en = EN,
            .valid_level = LOW,
        };
        
        channel_a_.pos_channel().init(pwm_noinv_cfg);
        channel_a_.neg_channel().init(pwm_noinv_cfg);
        channel_b_.pos_channel().init(pwm_inv_cfg);
        channel_b_.neg_channel().init(pwm_inv_cfg);

    }

    void set_dutycycle(const iq16 duty_a, const iq16 duty_b){
        channel_a_.set_dutycycle(duty_a);
        channel_b_.set_dutycycle(duty_b);
    }
private:

    hal::BipolarityTimerOcPair channel_a_;
    hal::BipolarityTimerOcPair channel_b_;
};

class RobotDynamics{

    void move(const Vec2<iq16> dir){
        static iq16 left = 0;
        static iq16 right = 0;

        left = (left*0.2_iq16 + CLAMP2(-dir.y,0.19_iq16)*0.8_iq16);
        right = (right*0.2_iq16 + CLAMP2(dir.x,0.19_iq16)*0.8_iq16);
        jetpwm_.set_dutycycle(left,right);
    }
private:
    JetPwm jetpwm_;
};

static constexpr iq16 DELTA = 0.01_iq16;


template<typename Derived>
struct TaskBase{
    struct Measurements{
        Vec2<iq16> angular_accel;
        Vec2<iq16> omega;
        Vec2<iq16> orientation;
    };

    using Output = Vec2<iq16>;
};

struct TaskCenter:public TaskBase<TaskCenter>{
    static constexpr iq16 Kp = 0.017_iq16;
    static constexpr iq16 Kd = 0.42_iq16;
    constexpr Output calc(const Measurements meas) const {
        return Kp*(-meas.angular_accel)+ Kd*(-meas.omega);
    }
};


struct TaskCircle:public TaskBase<TaskCircle>{
    static constexpr iq16 K_tan = 0.23_iq16;
    static constexpr iq16 Kp_norm = 2.2_iq16;
    static constexpr iq16 Kd_norm = 0.07_iq16;

    iq16 expected_iq16adius;
    constexpr Output calc(const Measurements meas) const {
        using namespace PhysicalConstants;

        const auto vec_norm = meas.orientation.normalized();//计算当前摆杆姿态的法向量
        const auto vec_tan = vec_norm.rotated(90_deg);//计算当前摆杆姿态的切向量
        
        const iq16 cmd_theta = std::atan2(W, H);
        const iq16 meas_theta = std::acos( //计算摆杆与铅垂线的角度
            std::cos(meas.orientation.x) * std::cos(meas.orientation.y));
        const iq16 theta_err = cmd_theta - meas_theta;

        const iq16 cmd_omega = std::sin(cmd_theta) * //期望的公转角速度
            std::sqrt(G / (x * std::cos(cmd_theta)));
        const iq16 omega_err = cmd_omega - std::abs(meas.omega.dot(vec_tan));

        const auto out_tan = K_tan * omega_err * vec_tan * //切向控制器输出
            sign(meas.omega.dot(vec_tan));

        const auto out_norm = //法向控制器输出
            (Kp_norm * theta_err - Kd_norm * meas.angular_accel.dot(vec_norm))
            * vec_norm * std::abs(theta_err);

        return out_norm + out_tan;
    }
};

struct TaskLine:public TaskBase<TaskLine>{
    constexpr Output calc(const Measurements meas, const Angle<iq16> theta){
        using namespace PhysicalConstants;

        const auto orientation = meas.orientation;
        const auto angular_accel = meas.angular_accel; 
        const auto omega = meas.omega;

        const Vec2 vec_norm = Vec2(0.0_iq16, 1.0_iq16).rotated(theta);
        const Vec2 vec_tan = Vec2(1.0_iq16, 0.0_iq16).rotated(theta);
        
        const iq16 d1 = meas.omega.cross(vec_tan);
        const iq16 d2 = meas.angular_accel.cross(vec_tan);
        const Vec2 out_norm = (0.027_iq16*d2 + 0.37_iq16*d1)*vec_norm;

        const iq16 E_targ = M * G * x * (1.0_iq16 - H / sqrt(H*H + W*W));
        
        const iq16 E_p = M * G * x * (1.0_iq16 - cos(orientation.x)*cos(orientation.y));
        const iq16 E_k = 0.5_iq16 * J * (omega.project(vec_tan)).length_squared();
        const iq16 E = E_p + E_k;

        const Vec2 out_tan = 15_iq16*(E_targ - E + 0.00037_iq16*abs(angular_accel.dot(vec_tan))) 
            * vec_tan * sign(omega.dot(vec_tan));

        return out_norm + out_tan;
    }
};
