#include <atomic>
#include <array>

#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/system.hpp"
#include "core/string/string_view.hpp"
#include "core/utils/bitflag.hpp"

#include "hal/timer/instance/timer_hw.hpp"
#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/can/can.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/bus/spi/spihw.hpp"
#include "hal/gpio/gpio.hpp"

#include "types/regions/range2.hpp"
#include "types/vectors/vector2.hpp"
#include "types/vectors/vector3.hpp"
#include "types/vectors/quat.hpp"

#include "robots/repl/repl_service.hpp"
#include "digipw/prelude/abdq.hpp"

using namespace ymd;

namespace PhysicalConstants {
    static constexpr real_t L_s = 0.098_r;
    static constexpr real_t L = 0.679_r;
    static constexpr real_t G = 9.8_r;
    static constexpr real_t h = 0.038_r;
    static constexpr real_t H = L + h;
    static constexpr real_t W = L + h;
    static constexpr real_t M1 = 0.07_r;
    static constexpr real_t M2 = 0.05_r;
    static constexpr real_t M = M1+M2;
    static constexpr real_t x = (M1 + 2*M2)/(2*M1 + 2*M2)*L;
    static constexpr real_t J = M1*L*L/3 + M2*L*L;
}

static constexpr real_t cali_r(const real_t _x){
    real_t x = CLAMP2(_x, 1.0_r);
    return sign(x)*(pow(abs(x), 1.5_r)+0.022_r);
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

    void set_dutycycle(const real_t duty_a, const real_t duty_b){
        channel_a_.set_dutycycle(duty_a);
        channel_b_.set_dutycycle(duty_b);
    }
private:

    hal::TimerOcPair channel_a_;
    hal::TimerOcPair channel_b_;
};

class RobotDynamics{

    void move(const Vec2<real_t> dir){
        static real_t left = 0;
        static real_t right = 0;

        left = (left*0.2_r + CLAMP2(-dir.y,0.19_r)*0.8_r);
        right = (right*0.2_r + CLAMP2(dir.x,0.19_r)*0.8_r);
        jetpwm_.set_dutycycle(left,right);
    }
private:
    JetPwm jetpwm_;
};

static constexpr real_t DELTA = 0.01_r;


template<typename Derived>
struct TaskBase{
    struct Measurements{
        Vec2<real_t> angular_accel;
        Vec2<real_t> omega;
        Vec2<real_t> orientation;
    };

    using Output = Vec2<real_t>;
};

struct TaskCenter:public TaskBase<TaskCenter>{
    static constexpr real_t Kp = 0.017_r;
    static constexpr real_t Kd = 0.42_r;
    constexpr Output calc(const Measurements meas) const {
        return Kp*(-meas.angular_accel)+ Kd*(-meas.omega);
    }
};


struct TaskCircle:public TaskBase<TaskCircle>{
    static constexpr real_t K_tan = 0.23_r;
    static constexpr real_t Kp_norm = 2.2_r;
    static constexpr real_t Kd_norm = 0.07_r;

    real_t expected_radius;
    constexpr Output calc(const Measurements meas) const {
        using namespace PhysicalConstants;

        const auto vec_norm = meas.orientation.normalized();//计算当前摆杆姿态的法向量
        const auto vec_tan = vec_norm.rotated(90_deg);//计算当前摆杆姿态的切向量
        
        const real_t cmd_theta = std::atan2(W, H);
        const real_t meas_theta = std::acos( //计算摆杆与铅垂线的角度
            std::cos(meas.orientation.x) * std::cos(meas.orientation.y));
        const real_t theta_err = cmd_theta - meas_theta;

        const real_t cmd_omega = std::sin(cmd_theta) * //期望的公转角速度
            std::sqrt(G / (x * std::cos(cmd_theta)));
        const real_t omega_err = cmd_omega - std::abs(meas.omega.dot(vec_tan));

        const auto out_tan = K_tan * omega_err * vec_tan * //切向控制器输出
            sign(meas.omega.dot(vec_tan));

        const auto out_norm = //法向控制器输出
            (Kp_norm * theta_err - Kd_norm * meas.angular_accel.dot(vec_norm))
            * vec_norm * std::abs(theta_err);

        return out_norm + out_tan;
    }
};

struct TaskLine:public TaskBase<TaskLine>{
    constexpr Output calc(const Measurements meas, const Angle<real_t> theta){
        using namespace PhysicalConstants;

        const auto orientation = meas.orientation;
        const auto angular_accel = meas.angular_accel; 
        const auto omega = meas.omega;

        const Vec2 vec_norm = Vec2(0.0_r, 1.0_r).rotated(theta);
        const Vec2 vec_tan = Vec2(1.0_r, 0.0_r).rotated(theta);
        
        const real_t d1 = meas.omega.cross(vec_tan);
        const real_t d2 = meas.angular_accel.cross(vec_tan);
        const Vec2 out_norm = (0.027_r*d2 + 0.37_r*d1)*vec_norm;

        const real_t E_targ = M * G * x * (1.0_r - H / sqrt(H*H + W*W));
        
        const real_t E_p = M * G * x * (1.0_r - cos(orientation.x)*cos(orientation.y));
        const real_t E_k = 0.5_r * J * (omega.project(vec_tan)).length_squared();
        const real_t E = E_p + E_k;

        const Vec2 out_tan = 15_r*(E_targ - E + 0.00037_r*abs(angular_accel.dot(vec_tan))) 
            * vec_tan * sign(omega.dot(vec_tan));

        return out_norm + out_tan;
    }
};
