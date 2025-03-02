#pragma once

#include "hal/gpio/gpio.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"

namespace gxm{
    
class Nozzle{
public:
    struct Config{
        // uint sustain;
    };
protected:
    const Config & config_;
    
    using GpioIntf = ymd::GpioIntf;
    using PwmChannelIntf = ymd::PwmChannelIntf;

    // GpioIntf & gpio_a_;
    // GpioIntf & gpio_b_;
    PwmChannelIntf & pwm_;

public:
    Nozzle(const Config & config, PwmChannelIntf & pwm):
        config_(config), pwm_(pwm){;}

    void release();
    void press();
    void off();
};
}