#pragma once


#include "hal/timer/timer_utils.hpp"
#include "concept/pwm_channel.hpp"


namespace ymd::hal{

class DmaChannel;

class TimerChannel{
public:
    using ChannelNth = TimerChannelNth;

    TimerChannel(const TimerChannel & other) = delete;
    TimerChannel(TimerChannel && other) = delete;

    TimerChannel & enable_dma(const Enable en = EN);

    DmaChannel & dma() const;

protected:
    TIM_TypeDef * inst_;

    const ChannelNth nth_;

    static volatile uint16_t & from_channel_to_cvr(
        TIM_TypeDef * timer, const ChannelNth nth);

    TimerChannel(TIM_TypeDef * inst, const ChannelNth nth):
        inst_(inst), 
        nth_(nth){;}
};

}