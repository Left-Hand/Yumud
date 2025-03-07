#pragma once

#include "hal/bus/bus.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{
class Smbus:public ProtocolBus{
protected:
    // hal::GpioIntf & scl_gpio;
    // hal::GpioIntf & sda_gpio;

    uint32_t timeout = 10;

    // I2c(hal::GpioIntf & _scl_gpio, hal::GpioIntf & _ada_gpio):scl_gpio(_scl_gpio),sda_gpio(_ada_gpio){}
public:
    void setTimeout(const uint32_t _timeout){timeout = _timeout;}

};


}