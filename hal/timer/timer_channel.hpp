#pragma once


#include "hal/timer/timer_utils.hpp"
#include "primitive/pwm_channel.hpp"
#include "core/utils/Option.hpp"


namespace ymd::hal{

class DmaChannel;

class TimerChannel{
public:
    using ChannelSelection = TimerChannelSelection;

    TimerChannel(const TimerChannel & other) = delete;
    TimerChannel(TimerChannel && other) = delete;

    void enable_dma(const Enable en);

    Option<DmaChannel &> dma() const;

protected:
    TIM_TypeDef * inst_;

    const ChannelSelection ch_sel_;

    static volatile uint16_t & from_channel_to_cvr(
        TIM_TypeDef * timer, const ChannelSelection ch_sel);

    TimerChannel(TIM_TypeDef * inst, const ChannelSelection ch_sel):
        inst_(inst), 
        ch_sel_(ch_sel){;}
};

}