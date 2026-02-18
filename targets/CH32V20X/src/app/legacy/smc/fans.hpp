#pragma once

#include "../hal/timer/timer.hpp"
#include "../drivers/Modem/dshot/dshot.hpp"

namespace SMC{

class SideFan{
protected:
    TimerOC & instanceP;
    TimerOC & instanceN;
    static constexpr iq16 duty_clamp = 0.92;
    static constexpr iq16 k = 6.5;

    bool enabled = true;
    iq16 last_force = 0;
    iq16 last_t = 0;
public:
    SideFan(TimerOC & _instanceP,TimerOC & _instanceN):instanceP(_instanceP), instanceN(_instanceN){;}

    void init(){
        instanceP.init();
        instanceN.init();
    }

    void reset(){
        last_force = 0;
        last_t = 0;
    }

    void enable(const Enable en){
        enabled = en;
        if(!en){
            instanceP = (iq16(0));
            instanceN = (iq16(0));
        }
    }

    void setDuty(const iq16 _duty){
        if(!enabled){
            return;
        }

        iq16 duty = CLAMP(_duty, -duty_clamp, duty_clamp);
        if(duty > 0){
            instanceP = (duty);
            instanceN = (iq16(0));
        }else{
            instanceP = (iq16(0));
            instanceN = (-duty);
        }
    }

    void setForce(const iq16 force){
        setDuty(SIGN_AS(sqrt(ABS(force)), force));
    }

    auto & operator = (const iq16 _force){

        iq16 step = k * (t - last_t);

        last_force = STEP_TO(last_force, _force, step);
        last_t = t;

        setForce(last_force);
        return *this;
    }
};


class HriFanPair{
protected:
    SideFan & left_fan;
    SideFan & right_fan;
public:

    HriFanPair(SideFan & _left_fan, SideFan & _right_fan):
        left_fan(_left_fan), right_fan(_right_fan){;}
    void setForce(const iq16 force){
        left_fan.setForce(-force);
        right_fan.setForce(force);
    }

    void enable(const Enable en){
        left_fan.start();
        right_fan.start();
    }

    void init(){
        left_fan.init();
        right_fan.init();
    }

    void reset(){
        left_fan.reset();
        right_fan.reset();
    }

    auto & operator = (const iq16 _force){
        setForce(_force);
        return *this;
    }
};

};
