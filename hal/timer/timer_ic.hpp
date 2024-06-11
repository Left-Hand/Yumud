
#ifndef __TIMER_IC_HPP__
#define __TIMER_IC_HPP__

#include "timer_channel.hpp"

class TimerIC:public TimerChannel{
public:
    enum class Channel:uint8_t{
        CH1CH2, CH3CH4
    };

protected:
    TIM_TypeDef * instance;
    volatile uint16_t & cvr1;
    volatile uint16_t & cvr2;

    const Channel channel;
public:
    
};
#endif