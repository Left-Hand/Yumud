#pragma once

#include "hal/gpio/gpio.hpp"

namespace ymd::drivers{

class KeyTrait{
    virtual bool pressed() = 0;
    virtual GpioConcept & io() = 0;
    virtual void update() = 0;
};

}