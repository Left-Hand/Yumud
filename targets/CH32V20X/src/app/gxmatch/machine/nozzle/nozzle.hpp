#pragma once

#include "hal/gpio/gpio.hpp"

namespace gxm{
    
class Nozzle{
protected:
    GpioConcept & gpio_a_;
    GpioConcept & gpio_b_;
public:
    Nozzle(GpioConcept & gpio_a, GpioConcept & gpio_b): gpio_a_(gpio_a), gpio_b_(gpio_b){;}

    void release();
    void press();
    void off();
};
}