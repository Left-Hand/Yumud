#pragma once

#include "drivers/Actuator/servo/servo.hpp"
#include "algebra/regions/range2.hpp"
#include "primitive/pwm_channel.hpp"


namespace ymd::drivers{

class ScaledPwm final:public hal::PwmIntf{

public:
    ScaledPwm(
        hal::PwmIntf & pwm, 
        const math::Range2<uq16> & dutycycle_range
    ):
        pwm_(pwm), 
        dutycycle_range_(dutycycle_range){;}

    void enable(const Enable en){
        enabled_ = en == EN;
        if(en == DISEN) this->set_dutycycle(0);
    }

    void set_dutycycle(const uq16 dutycycle) {
        if(false == enabled_){
            pwm_.set_dutycycle(0);
        }else{
            pwm_.set_dutycycle(dutycycle_range_.lerp(dutycycle));
        }
    }

    auto & inst(){
        return pwm_;
    }
protected:
    hal::PwmIntf & pwm_;
    math::Range2<uq16> dutycycle_range_;
    bool enabled_ = true;
};


class PwmServo final:public ServoBase{
private:
    ScaledPwm pwm_;
    Angular<iq16> last_angle;

    void set_global_angle(const Angular<iq16> angle) {
        pwm_.set_dutycycle((angle).to_turns());
        last_angle = angle;
    }

    Angular<iq16> get_global_angle() {
        return last_angle;
    }
    
public:
    PwmServo(hal::PwmIntf & pwm):
        pwm_(pwm, {iq16(0.025), iq16(0.125)})
        {;}

};

class PwmSpeedServo:public SpeedServo{
protected: 
    ScaledPwm pwm_;
    iq16 max_turns_per_second_;
    iq16 expect_speed_;

    void set_speed_directly(const iq16 rps) {
        expect_speed_ = rps;
        set_dutycycle(rps / max_turns_per_second_);
    }

    iq16 get_speed() {
        return expect_speed_;
    }

    void set_dutycycle(const iq16 dutycycle){
        pwm_.set_dutycycle((dutycycle + 1) * iq16(0.5));
    }
public:
    PwmSpeedServo(hal::PwmIntf & pwm, const iq16 max_turns_per_second = 2):
            pwm_(pwm, {iq16(0.025), iq16(0.125)}),
            max_turns_per_second_(max_turns_per_second),
            expect_speed_(iq16(0))
            {;}


};

using MG995 = PwmServo;
using SG90 = PwmServo;
}