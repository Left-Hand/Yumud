#pragma once

#include "hal/bus/bus.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{
class Pmbus:public ProtocolBus{
protected:
    // GpioConcept & scl_gpio;
    // GpioConcept & sda_gpio;

    uint32_t timeout = 10;

    // I2c(GpioConcept & _scl_gpio, GpioConcept & _ada_gpio):scl_gpio(_scl_gpio),sda_gpio(_ada_gpio){}
public:
    void setTimeout(const uint32_t _timeout){timeout = _timeout;}

};


}