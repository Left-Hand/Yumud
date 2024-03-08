#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "timer_oc.hpp"
#include "types/real.hpp"

class PwmChannelBase{
protected:
    real_t min_value = real_t(0);
    real_t max_value = real_t(1);
public:
    void setClamp(const real_t & abs_max_value){
        min_value = real_t(0);
        max_value = abs(abs_max_value);
    }
    void setClamp(const real_t & _min_value, const real_t & _max_value){
        min_value = MAX(_min_value, real_t(0));
        max_value = MIN(_max_value, real_t(1));
    }

    void init(const real_t & abs_max_value = real_t(1)) {setClamp(abs_max_value);}
    void init(const real_t & _min_value, const real_t & _max_value){setClamp(_min_value, _max_value);}

    virtual void setDuty(const real_t & duty) = 0;
};

class PwmChannel:public PwmChannelBase{
protected:
    TimerOC & instance;

public:
    PwmChannel(TimerOC & _pwm) : instance(_pwm) {;}

    __fast_inline void setDuty(const real_t & duty) override{
        instance = uint16_t(CLAMP(duty, min_value, max_value) * instance.getPreloadData());
    }

    __fast_inline PwmChannel & operator = (const real_t & duty){setDuty(duty); return *this;}
};

#endif