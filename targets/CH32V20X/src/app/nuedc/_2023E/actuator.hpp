#pragma once

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"


#include "primitive/pwm_channel.hpp"
#include "algebra/vectors/vec2.hpp"
#include "algebra/vectors/vec2.hpp"
#include "dsp/filter/firstorder/lpf.hpp"

#include "robots/mock/mock_servo.hpp"
#include "prelude.hpp"

using namespace ymd;


namespace nuedc::_2023E{

using ymd::robots::mock::MotorCmd;

struct ServoConfig{
    Angular<iq16> min_angle;
    Angular<iq16> max_angle;
};
// class PwmServo final:public MotorIntf{
class PwmServo final{
public:
    using Config = ServoConfig;

    explicit PwmServo(
        const Config & cfg, 
        iq16 min_duty, 
        iq16 max_duty, 
        ymd::hal::PwmIntf & pwm
    ) 
        :min_duty_(min_duty), max_duty_(max_duty), pwm_(pwm){
            reconf(cfg);
        }

    void reconf(const Config & cfg){
        min_angle_ = cfg.min_angle;
        max_angle_ = cfg.max_angle;
    }

    void set_motorcmd(const MotorCmd & cmd){
        set_angle(Angular<iq16>::from_turns(cmd.ref_x1));
    }
    void set_angle(const Angular<iq16> angle){
        ASSERT(min_angle_ <= angle, "angle out of range");
        ASSERT(angle <= max_angle_, "angle out of range");
        const auto dutycycle = LERP(INVLERP(
            angle.to_turns(), min_angle_.to_turns(), max_angle_.to_turns())
        , min_duty_, max_duty_);
        pwm_.set_dutycycle(dutycycle);
    }
private:
    iq16 min_duty_;
    iq16 max_duty_;
    Angular<iq16> min_angle_;
    Angular<iq16> max_angle_;
    ymd::hal::PwmIntf & pwm_;
};

PwmServo make_sg90(const ServoConfig & cfg, hal::PwmIntf & pwm){
    return PwmServo(cfg, 0.025_r, 0.125_r, pwm);
}
PwmServo make_mg995(const ServoConfig & cfg, hal::PwmIntf & pwm){
    return PwmServo(cfg, 0.025_r, 0.125_r, pwm);
}

//执行器 封装了对不同执行器的调用 屏蔽底层差异
class GimbalActuatorIntf{
public:
    virtual void set_gest(const GimbalSolution solu) = 0;
};


class GimbalActuatorByMock  final :public GimbalActuatorIntf {
public:
    void set_gest(const GimbalSolution solu){
        DEBUG_PRINTLN(solu);
    }
};

class GimbalActuatorByLambda  final :public GimbalActuatorIntf {
public:
    using Setter = std::function<void(MotorCmd)>;

    struct Params{
        Setter yaw_setter;
        Setter pitch_setter;
    };


    GimbalActuatorByLambda (const Params && params) :
        yaw_setter_     (std::move(params.yaw_setter)), 
        pitch_setter_   (std::move(params.pitch_setter))
    {}

    void set_gest(const GimbalSolution solu){
        yaw_setter_     ({.ref_x1 = solu.yaw, .ref_x2 = 0});
        pitch_setter_   ({.ref_x1 = solu.pitch, .ref_x2 = 0});
    }
private:
    Setter yaw_setter_;
    Setter pitch_setter_;
};


}