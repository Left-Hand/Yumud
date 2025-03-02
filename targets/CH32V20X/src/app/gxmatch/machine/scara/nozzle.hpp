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
    using PwmIntf = ymd::PwmIntf;

    // hal::GpioIntf & gpio_a_;
    // hal::GpioIntf & gpio_b_;
    PwmIntf & pwm_;

public:
    Nozzle(const Config & config, PwmIntf & pwm):
        config_(config), pwm_(pwm){;}

    void release();
    void press();
    void off();
};
}