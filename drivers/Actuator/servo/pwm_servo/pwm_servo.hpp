#pragma once

#include "drivers/Actuator/servo/servo.hpp"
#include "types/regions/range2.hpp"
#include "concept/pwm_channel.hpp"

namespace ymd::drivers{

class ScaledPwm final:public hal::PwmIntf{

public:
    ScaledPwm(
        hal::PwmIntf & pwm, 
        const Range2<real_t> & duty_range
    ):
        pwm_(pwm), 
        duty_range_(duty_range){;}

    void enable(const Enable en = EN){
        enabled_ = en == EN;
        if(en == DISEN) this->set_dutycycle(0);
    }

    void set_dutycycle(const real_t duty) override {
        if(false == enabled_){
            pwm_.set_dutycycle(0);
        }else{
            pwm_.set_dutycycle(duty_range_.lerp(duty));
        }
    }

    auto & inst(){
        return pwm_;
    }
protected:
    hal::PwmIntf & pwm_;
    Range2<real_t> duty_range_;
    bool enabled_ = true;
};


class PwmServo final:public ServoBase{
private:
    ScaledPwm pwm_;
    real_t last_angle;

    void set_global_angle(const real_t angle) override{
        pwm_.set_dutycycle((angle) * real_t(1 / PI));
        last_angle = angle;
    }

    real_t get_global_angle() override{
        return last_angle;
    }
    
public:
    PwmServo(hal::PwmIntf & pwm):
        pwm_(pwm, {real_t(0.025), real_t(0.125)})
        {;}

};

class PwmSpeedServo:public SpeedServo{
protected: 
    ScaledPwm pwm_;
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
        pwm_.set_dutycycle((duty + 1) * real_t(0.5));
    }
public:
    PwmSpeedServo(hal::PwmIntf & pwm, const real_t max_turns_per_second = 2):
            pwm_(pwm, {real_t(0.025), real_t(0.125)}),
            max_turns_per_second_(max_turns_per_second),
            expect_speed_(real_t(0))
            {;}


};

using MG995 = PwmServo;
using SG90 = PwmServo;
}