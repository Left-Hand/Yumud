#pragma once

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"



#include "drivers/GateDriver/AT8222/at8222.hpp"

#include "concept/pwm_channel.hpp"
#include "types/vectors/vector2/vector2.hpp"
#include "types/vectors/vector2/vector2.hpp"
#include "dsp/filter/rc/LowpassFilter.hpp"

#include "robots/mock/mock_servo.hpp"
#include "prelude.hpp"

using namespace ymd;
using namespace ymd::hal;

namespace nuedc::_2023E{

using ymd::robots::mock::MotorCmd;

struct ServoConfig{
    real_t min_radian;
    real_t max_radian;
};
// class PwmServo final:public MotorIntf{
class PwmServo final{
public:
    using Config = ServoConfig;

    PwmServo(
        const Config & cfg, 
        real_t min_duty, 
        real_t max_duty, 
        ymd::hal::PwmIntf & pwm
    ) 
        :min_duty_(min_duty), max_duty_(max_duty), pwm_(pwm){
            reconf(cfg);
        }

    void reconf(const Config & cfg){
        min_radian_ = cfg.min_radian;
        max_radian_ = cfg.max_radian;
    }

    void set_motorcmd(const MotorCmd & cmd){
        set_radian(cmd.ref_pos * real_t(PI));
    }
    void set_radian(const real_t radian){
        ASSERT(min_radian_ <= radian, "radian out of range");
        ASSERT(radian <= max_radian_, "radian out of range");
        const auto duty = LERP(INVLERP(radian, min_radian_, max_radian_), min_duty_, max_duty_);
        pwm_ = duty;
    }
private:
    real_t min_duty_;
    real_t max_duty_;
    real_t min_radian_;
    real_t max_radian_;
    ymd::hal::PwmIntf & pwm_;
};

PwmServo make_sg90(const ServoConfig & cfg, ymd::hal::PwmIntf & pwm){
    return PwmServo(cfg, 0.025_r, 0.125_r, pwm);
}
PwmServo make_mg995(const ServoConfig & cfg, ymd::hal::PwmIntf & pwm){
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
        yaw_setter_     ({.ref_pos = solu.yaw, .ref_spd = 0});
        pitch_setter_   ({.ref_pos = solu.pitch, .ref_spd = 0});
    }
private:
    Setter yaw_setter_;
    Setter pitch_setter_;
};


}