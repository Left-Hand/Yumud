#ifndef __TIMER_OC_HPP__
#define __TIMER_OC_HPP__

#include "pwm_channel.hpp"
#include "real.hpp"

class PwmChannel:public PwmChannelBase{
};

class TimerOC:public PwmChannelBase{
public:
    enum class Channel:uint8_t{
        CH1, CH1N, CH2, CH2N, CH3, CH3N, CH4
    };

protected:
    TIM_TypeDef * base;
    volatile uint16_t & cvr;
    // const uint8_t * data;
    const Channel channel;

    volatile uint16_t * from_channel_to_cvr(const Channel _channel);
public:


    TimerOC(TIM_TypeDef * _base, const Channel _channel);

    void init();

    void enable(const bool en = true) override;
    // void 

    void setDuty(const real_t & duty) override{
        *this = (uint16_t)(int)(duty * base->CNT);
    }

    TimerOC & operator = (const uint16_t _val){cvr = _val;return *this;}
    TimerOC & operator = (const bool en){enable(en); return *this;}

};
#endif