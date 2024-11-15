#pragma once

#include "hal/gpio/port.hpp"
#include "hal/timer/timer_utils.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/dma/dma.hpp"

namespace ymd{

class TimerChannel{
public:
    using Channel = TimerUtils::Channel;

    void enableDma(const bool en = true);

    DmaChannel & dma() const;
protected:
    TIM_TypeDef * instance;
    const Channel channel;

    volatile uint16_t & from_channel_to_cvr(const Channel _channel);

    TimerChannel(TIM_TypeDef * _instance, const Channel _channel):instance(_instance), channel(_channel){;}

};

}