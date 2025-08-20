#pragma once

#include "drivers/Actuator/servo/servo.hpp"
#include "types/regions/range2.hpp"
#include "concept/pwm_channel.hpp"

namespace ymd::drivers{

class ScaledPwm final:public hal::PwmIntf{
protected:
    hal::PwmIntf & instance_;
    Range2<real_t> duty_range_;
    bool enabled = true;
public:
    ScaledPwm(
        hal::PwmIntf & instance, 
        const Range2<real_t> & duty_range
    ):
        instance_(instance), 
        duty_range_(duty_range){;}

    void enable(const Enable en = EN){
        enabled = en == EN;
        if(en == DISEN) this->set_dutycycle(0);
    }

    void set_dutycycle(const real_t duty) override {
        if(false == enabled){
            instance_ = 0;
        }else{
            instance_ = duty_range_.lerp(duty);
        }
    }

    auto & inst(){
        return instance_;
    }
};


class PwmRadianServo final:public RadianServoBase{
private:
    ScaledPwm instance_;
    real_t last_rad;

    void set_global_radian(const real_t rad) override{
        instance_.set_dutycycle((rad) * real_t(1 / PI));
        last_rad = rad;
    }

    real_t get_global_radian() override{
        return last_rad;
    }
    
public:
    PwmRadianServo(hal::PwmIntf & instance):
        instance_(instance, {real_t(0.025), real_t(0.125)})
        {;}

};

class PwmSpeedServo:public SpeedServo{
protected: 
    ScaledPwm instance_;
    real_t max_turns_per_second_;
    real_t expect_speed_;

    void set_speed_directly(const real_t rps) override{
        expect_speed_ = rps;
        set_dutycycle(rps / max_turns_per_second_);
    }

    real_t get_speed() override{
        return expect_speed_;
    }

    void set_dutycycle(const real_t duty){
        instance_.set_dutycycle((duty + 1) * real_t(0.5));
    }
public:
    PwmSpeedServo(hal::PwmIntf & instance, const real_t max_turns_per_second = 2):
            instance_(instance, {real_t(0.025), real_t(0.125)}),
            max_turns_per_second_(max_turns_per_second),
            expect_speed_(real_t(0))
            {;}


};

using MG995 = PwmRadianServo;
using SG90 = PwmRadianServo;
}