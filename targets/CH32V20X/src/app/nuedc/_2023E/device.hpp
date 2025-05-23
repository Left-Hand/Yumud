#pragma once

#include "core/math/realmath.hpp"
#include "core/system.hpp"
#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/timer.hpp"
#include "hal/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"


#include "drivers/Actuator/Bridge/AT8222/at8222.hpp"


#include "concept/pwm_channel.hpp"
#include "types/vector2/vector2.hpp"

using namespace ymd;
using namespace ymd::hal;

namespace nuedc::_2023E{

struct ServoConfig{
    real_t min_radian;
    real_t max_radian;
};

struct MotorCmd{
    real_t ref_pos;
    real_t ref_spd;
};

class MotorIntf{
    virtual void set_motorcmd(const MotorCmd & cmd);
};

class PwmServo final:public MotorIntf{
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

    static PwmServo make_sg90(const Config & cfg, ymd::hal::PwmIntf & pwm){
        return PwmServo(cfg, 0.025_r, 0.125_r, pwm);
    }
    static PwmServo make_mg995(const Config & cfg, ymd::hal::PwmIntf & pwm){
        return PwmServo(cfg, 0.025_r, 0.125_r, pwm);
    }
private:
    real_t min_duty_;
    real_t max_duty_;
    real_t min_radian_;
    real_t max_radian_;
    ymd::hal::PwmIntf & pwm_;
};


}