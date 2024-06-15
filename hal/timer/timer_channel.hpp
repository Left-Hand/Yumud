#pragma once

#include "hal/gpio/port.hpp"
#include "hal/timer/timer_utils.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"



class TimerChannel{
public:
    using Channel = TimerUtils::Channel;

protected:
    TIM_TypeDef * instance;
    const Channel channel;

    volatile uint16_t & from_channel_to_cvr(const Channel _channel);

    TimerChannel(TIM_TypeDef * _instance, const Channel _channel):instance(_instance), channel(_channel){;}
};
