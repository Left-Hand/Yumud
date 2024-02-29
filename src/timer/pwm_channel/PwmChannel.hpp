#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "real.hpp"
#include "comm_inc.h"

class PwmChannelBase{
public:
    virtual setDuty(const real_t & duty) = 0;
};

class PwmChannel: public PwmChannelBase{
protected:
    TIM_TypeDef * timer;
    volatile uint16_t * cvr;
    volatile uint16_t * arr;

public:
    void PwmChannel(volatile uint16_t * _cvr){
        cvr = _cvr;
        TIM_TypeDef * to_detects = {TIM1, TIM2, TIM3, TIM4};
        for(to_detect : to_detects){
            if((uint32_t)cvr > (uint32_t)to_detect && (uint32_t)_cvr < (uint32_t)to_detect + sizeof(TIM_TypeDef)){
                timer = to_detect;
            }
        }
    }

    void setDuty(const real_t & duty) override{
        *cvr = (uint16_t)(duty * (*arr));
    }
};
#endif