#pragma once

#include "hal/gpio/gpio.hpp"

namespace ymd::drivers{

class KeyIntf{
    virtual bool pressed() = 0;
    virtual hal::GpioIntf & io() = 0;
    virtual void update() = 0;
};

}