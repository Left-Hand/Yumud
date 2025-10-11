#pragma once

#include <chrono>

#include "hal/bus/bus_base.hpp"
#include "core/utils/Option.hpp"

#include "i2c_utils.hpp"

namespace ymd::hal{
class Gpio;

class I2c{
public:
    I2c(I2c && other) = default;

    struct Config{
        uint32_t baudrate;
    };

    void set_timeout(const std::chrono::microseconds timeout){timeout_ = timeout;}
    void discard_ack(const Enable en){discard_ack_ = en == EN;}

    virtual hal::HalResult read(uint32_t & data, const Ack ack) = 0;
    virtual hal::HalResult write(const uint32_t data) = 0;
    virtual hal::HalResult unlock_bus() = 0;
    virtual hal::HalResult set_baudrate(const uint32_t baudrate) = 0;

    __fast_inline hal::Gpio & scl(){return scl_gpio_;};
    __fast_inline hal::Gpio & sda(){return sda_gpio_;};

    virtual HalResult lead(const I2cSlaveAddrWithRw req) = 0;
    virtual void trail() = 0;
    HalResult borrow(const I2cSlaveAddrWithRw req);

    void lend(){
        this->trail();
        owner_.lend();
    }

    bool is_occupied(){return owner_.is_borrowed();}

    struct Guard {
        I2c & i2c_;
        Guard(I2c & i2c):i2c_(i2c){}
        ~Guard(){i2c_.lend();}
    };

    auto create_guard(){return Guard(*this);}
protected:
    using Timeout = std::chrono::duration<uint16_t, std::micro>;
    Timeout timeout_ = Timeout(10);
    bool discard_ack_ = false;

    I2c(Some<hal::Gpio *> scl_gpio, Some<hal::Gpio *> sda_gpio):
        scl_gpio_(scl_gpio.deref()),sda_gpio_(sda_gpio.deref()){}
private:
    hal::Gpio & scl_gpio_;
    hal::Gpio & sda_gpio_;

    PeripheralOwnershipTracker owner_ = {};
};


}