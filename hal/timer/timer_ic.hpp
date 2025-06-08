#pragma once

#include "timer_channel.hpp"

namespace ymd{

class TimerIC:public TimerChannel{
public:
    enum class Channel:uint8_t{
        CH1CH2, CH3CH4
    };

protected:
    TIM_TypeDef * inst_;
    volatile uint16_t & cvr1_;
    volatile uint16_t & cvr2_;

    const Channel channel_;
public:
    
};

};
