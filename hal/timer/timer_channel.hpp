#pragma once


#include "hal/timer/timer_utils.hpp"
#include "concept/pwm_channel.hpp"


namespace ymd::hal{

class DmaChannel;

class TimerChannel{
public:
    using ChannelSelection = TimerChannelSelection;

    TimerChannel(const TimerChannel & other) = delete;
    TimerChannel(TimerChannel && other) = delete;

    TimerChannel & enable_dma(const Enable en);

    DmaChannel & dma() const;

protected:
    TIM_TypeDef * inst_;

    const ChannelSelection nth_;

    static volatile uint16_t & from_channel_to_cvr(
        TIM_TypeDef * timer, const ChannelSelection nth);

    TimerChannel(TIM_TypeDef * inst, const ChannelSelection nth):
        inst_(inst), 
        nth_(nth){;}
};

}