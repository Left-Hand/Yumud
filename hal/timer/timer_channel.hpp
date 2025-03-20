#pragma once


#include "hal/timer/timer_utils.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/dma/dma.hpp"

namespace ymd::hal{

class TimerChannel{
public:
    using ChannelIndex = TimerChannelIndex;
protected:
    TIM_TypeDef * instance;

    const ChannelIndex idx_;

    static volatile uint16_t & from_channel_to_cvr(TIM_TypeDef * timer, const ChannelIndex _channel);
    TimerChannel(TIM_TypeDef * _instance, const ChannelIndex idx):
        instance(_instance), 
        idx_(idx){;}
public:
    TimerChannel(const TimerChannel & other) = delete;
    TimerChannel(TimerChannel && other) = delete;

    TimerChannel & enableDma(const bool en = true);

    DmaChannel & dma() const;
};

}