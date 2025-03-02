#pragma once

#include "hal/bus/bus.hpp"
#include "hal/gpio/gpio.hpp"
#include "i2c_trait.hpp"

namespace ymd::hal{
class I2c:public HalfDuplexBus{
protected:
    hal::GpioIntf & scl_gpio;
    hal::GpioIntf & sda_gpio;

    uint32_t timeout_ = 10;

    I2c(hal::GpioIntf & _scl_gpio, hal::GpioIntf & _ada_gpio):scl_gpio(_scl_gpio),sda_gpio(_ada_gpio){}
public:
    void setTimeout(const uint32_t timeout){timeout_ = timeout;}

    hal::GpioIntf & scl(){return scl_gpio;};
    hal::GpioIntf & sda(){return sda_gpio;};
};


}