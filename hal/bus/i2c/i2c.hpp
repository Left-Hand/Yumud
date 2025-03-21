#pragma once

#include "hal/bus/bus_base.hpp"
#include "i2c_trait.hpp"

namespace ymd::hal{
class Gpio;

class I2c:public BusBase{
protected:
    hal::Gpio & scl_gpio;
    hal::Gpio & sda_gpio;

    uint32_t timeout_ = 10;

    I2c(hal::Gpio & _scl_gpio, hal::Gpio & _ada_gpio):scl_gpio(_scl_gpio),sda_gpio(_ada_gpio){}
public:
    void set_timeout(const uint32_t timeout){timeout_ = timeout;}

    virtual BusError read(uint32_t & data, const Ack ack) = 0;
    virtual BusError write(const uint32_t data) = 0;

    hal::Gpio & scl(){return scl_gpio;};
    hal::Gpio & sda(){return sda_gpio;};

    virtual void set_baudrate(const uint32_t baudrate) = 0;
};


}