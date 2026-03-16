#pragma once


#include "hal/timer/timer_lld.hpp"
#include "primitive/pwm_channel.hpp"
#include "core/utils/Option.hpp"


namespace ymd::hal{

class DmaChannel;

class [[nodiscard]] TimerChannel{
public:
    using ChannelSelection = TimerChannelSelection;

    TimerChannel(const TimerChannel & other) = delete;
    TimerChannel(TimerChannel && other) = delete;

    void enable_dma(const Enable en);

    Option<DmaChannel &> dma() const;

protected:
    void * p_inst_;

    const ChannelSelection sel_;

    static volatile uint16_t & from_channel_to_cvr(
        void * timer, const ChannelSelection sel);

    TimerChannel(void * p_inst, const ChannelSelection sel):
        p_inst_(p_inst), 
        sel_(sel){;}
};

}