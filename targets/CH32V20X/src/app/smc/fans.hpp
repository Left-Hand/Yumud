#ifndef __SMC_FANS_HPP__

#define __SMC_FANS_HPP__

#include "../hal/timer/timer.hpp"
#include "../drivers/Modem/dshot/dshot.hpp"

namespace SMC{

class SideFan{
protected:
    TimerOC & instanceP;
    TimerOC & instanceN;
    static constexpr real_t duty_clamp = 0.92;
    static constexpr real_t k = 6.5;

    bool enabled = true;
    real_t last_force = 0;
    real_t last_t = 0;
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

    void enable(const bool en = true){
        enabled = en;
        if(!en){
            instanceP = (real_t(0));
            instanceN = (real_t(0));
        }
    }

    void setDuty(const real_t _duty){
        if(!enabled){
            return;
        }

        real_t duty = CLAMP(_duty, -duty_clamp, duty_clamp);
        if(duty > 0){
            instanceP = (duty);
            instanceN = (real_t(0));
        }else{
            instanceP = (real_t(0));
            instanceN = (-duty);
        }
    }

    void setForce(const real_t force){
        setDuty(SIGN_AS(sqrt(ABS(force)), force));
    }

    auto & operator = (const real_t _force){

        real_t step = k * (t - last_t);

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
    void setForce(const real_t force){
        left_fan.setForce(-force);
        right_fan.setForce(force);
    }

    void enable(const bool & en = true){
        left_fan.enable(en);
        right_fan.enable(en);
    }

    void init(){
        left_fan.init();
        right_fan.init();
    }

    void reset(){
        left_fan.reset();
        right_fan.reset();
    }

    auto & operator = (const real_t _force){
        setForce(_force);
        return *this;
    }
};

};

#endif