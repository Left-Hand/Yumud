#pragma once

#include "hal/bus/bus.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{
class Smbus:public ProtocolBus{
protected:
    // hal::GpioIntf & scl_pin;
    // hal::GpioIntf & sda_pin;

    uint32_t timeout = 10;

    // I2cBase(hal::GpioIntf & _scl_pin, hal::GpioIntf & _ada_gpio):scl_pin(_scl_pin),sda_pin(_ada_gpio){}
public:
    void setTimeout(const uint32_t _timeout){timeout = _timeout;}

};


}