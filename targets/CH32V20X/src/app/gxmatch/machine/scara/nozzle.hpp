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
    
    using GpioConcept = ymd::GpioConcept;
    using PwmChannel = ymd::PwmChannel;

    // GpioConcept & gpio_a_;
    // GpioConcept & gpio_b_;
    PwmChannel & pwm_;

public:
    Nozzle(const Config & config, PwmChannel & pwm):
        config_(config), pwm_(pwm){;}

    void release();
    void press();
    void off();
};
}