#pragma once

#include <chrono>

#include "hal/bus/bus_base.hpp"
#include "core/utils/Option.hpp"

#include "i2c_primitive.hpp"

namespace ymd::hal{
class Gpio;

class I2cBase{
public:
    using Timeout = std::chrono::duration<uint16_t, std::micro>;

    I2cBase(I2cBase && other) = default;

    struct Config{
        uint32_t baudrate;
    };



    virtual HalResult read(uint8_t & data, const Ack ack) = 0;
    virtual HalResult write(const uint32_t data) = 0;
    virtual HalResult unlock_bus() = 0;
    virtual HalResult set_baudrate(const uint32_t baudrate) = 0;

    virtual HalResult lead(const I2cSlaveAddrWithRw req) = 0;
    virtual void trail() = 0;

    virtual void lend() = 0;
    virtual HalResult borrow(const I2cSlaveAddrWithRw req);


    struct Guard {
        I2cBase & i2c_;
        Guard(I2cBase & i2c):i2c_(i2c){}
        ~Guard(){i2c_.lend();}
    };

    auto create_guard(){return Guard(*this);}
protected:

    I2cBase() = default;
};


}