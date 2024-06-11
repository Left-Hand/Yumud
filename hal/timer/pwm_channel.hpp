#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "timer_oc.hpp"
#include "types/real.hpp"

class PwmChannelConcept{
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

    virtual void init() = 0;

    virtual void setDuty(const real_t & duty) = 0;

    virtual PwmChannelConcept & operator = (const real_t & duty) = 0;
};

class PwmChannel:public PwmChannelConcept{
protected:
    TimerOutChannelPosOnChip & instance;

public:
    PwmChannel(TimerOutChannelPosOnChip & _pwm) : instance(_pwm) {;}

    void init() override{
        instance.init();
    }
    __fast_inline void setDuty(const real_t & duty) override{
        instance = CLAMP(duty, min_value, max_value);
    }

    __fast_inline PwmChannel & operator = (const real_t & duty){setDuty(duty); return *this;}
    operator real_t() const {return real_t(instance);}
};

#endif