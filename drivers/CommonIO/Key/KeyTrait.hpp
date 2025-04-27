#pragma once

#include "hal/gpio/gpio.hpp"

namespace ymd::drivers{

class KeyIntf{
    virtual bool is_pressed() const = 0;
    virtual bool just_pressed() const = 0;
    virtual void update() = 0;
};

}