#pragma once

#include "hal/gpio/gpio.hpp"

namespace gxm{
    
class Nozzle{
public:
    struct Config{
    };
protected:
    const Config & config_;
    
    using GpioConcept = yumud::GpioConcept;

    GpioConcept & gpio_a_;
    GpioConcept & gpio_b_;

public:
    Nozzle(const Config & config, GpioConcept & gpio_a, GpioConcept & gpio_b):
        config_(config), gpio_a_(gpio_a), gpio_b_(gpio_b){;}

    void release();
    void press();
    void off();
};
}