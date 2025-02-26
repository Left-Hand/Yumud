#pragma once

#include "hal/bus/bus.hpp"
#include "hal/gpio/gpio.hpp"
#include "i2c_trait.hpp"

namespace ymd::hal{
class I2c:public HalfDuplexBus{
protected:
    GpioConcept & scl_gpio;
    GpioConcept & sda_gpio;

    uint32_t timeout_ = 10;

    I2c(GpioConcept & _scl_gpio, GpioConcept & _ada_gpio):scl_gpio(_scl_gpio),sda_gpio(_ada_gpio){}
public:
    void setTimeout(const uint32_t timeout){timeout_ = timeout;}

    GpioConcept & scl(){return scl_gpio;};
    GpioConcept & sda(){return sda_gpio;};
};


}