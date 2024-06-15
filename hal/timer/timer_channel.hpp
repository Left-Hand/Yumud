#pragma once

#include "hal/gpio/port.hpp"
#include "hal/timer/timer_utils.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

class TimerChannel{
public:
    enum class Channel:uint8_t{
        CH1, CH1N, CH2, CH2N, CH3, CH3N, CH4
    };

protected:
    TIM_TypeDef * instance;
    const Channel channel;

    volatile uint16_t & from_channel_to_cvr(const Channel _channel);
    Gpio & getPin();

    TimerChannel(TIM_TypeDef * _instance, const Channel _channel):instance(_instance), channel(_channel){;}
};
