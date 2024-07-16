#ifndef __SMC_FANS_HPP__

#define __SMC_FANS_HPP__

#include "../hal/timer/timer.hpp"
#include "../drivers/Modem/dshot/dshot.hpp"

namespace SMC{

class SideFan{
protected:
    TimerOC & instanceP;
    TimerOC & instanceN;
    Range_t<real_t>duty_clamp = {-0.7, 0.7};

    bool enabled = true;
    real_t last_force = 0;
public:
    SideFan(TimerOC & _instanceP,TimerOC & _instanceN):instanceP(_instanceP), instanceN(_instanceN){;}

    void init(){
        instanceP.init();
        instanceN.init();
    }

    void setClamp(const Range_t<real_t> & _duty_clamp){
        duty_clamp = _duty_clamp;
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

        real_t duty = duty_clamp.clamp(_duty);
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
        static constexpr real_t step = 0.03;
        last_force = STEP_TO(last_force, _force, step);
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

    auto & operator = (const real_t _force){
        setForce(_force);
        return *this;
    }
};

};

#endif