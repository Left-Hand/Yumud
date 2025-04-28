#pragma once

#include "hal/bus/bus_base.hpp"
#include "i2c_trait.hpp"

namespace ymd::hal{
class Gpio;

class I2c:public BusBase{
private:
    hal::Gpio & scl_gpio_;
    hal::Gpio & sda_gpio_;
    
protected:
    uint32_t timeout_ = 10;
    bool discard_ack_ = false;

    I2c(hal::Gpio & scl_gpio, hal::Gpio & sda_gpio):scl_gpio_(scl_gpio),sda_gpio_(sda_gpio){}
public:
    I2c(I2c && other) = default;

    void set_timeout(const uint32_t timeout){timeout_ = timeout;}
    void discard_ack(const bool en){discard_ack_ = en;}

    virtual hal::HalResult read(uint32_t & data, const Ack ack) = 0;
    virtual hal::HalResult write(const uint32_t data) = 0;
    virtual hal::HalResult unlock_bus() = 0;
    virtual hal::HalResult set_baudrate(const uint32_t baudrate) = 0;

    __fast_inline hal::Gpio & scl(){return scl_gpio_;};
    __fast_inline hal::Gpio & sda(){return sda_gpio_;};

};


}