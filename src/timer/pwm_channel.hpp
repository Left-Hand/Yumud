#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "timer_oc.hpp"

class PwmChannelBase{
public:

    virtual void setDuty(const real_t & duty) = 0;
};

class PwmChannel:public PwmChannelBase{
protected:
    TimerOC pwm;
    real_t min_value;
    real_t max_value;
public:
    PwmChannel(const TimerOC & _pwm) : pwm(_pwm) {;}

    void setClamp(const real_t & _min_value, const real_t & _max_value){
        min_value = _min_value;
        max_value = _max_value;
    }
    void setDuty(const real_t & duty) override{
        pwm.setDuty(CLAMP(duty, min_value, max_value));
    }
}

#endif