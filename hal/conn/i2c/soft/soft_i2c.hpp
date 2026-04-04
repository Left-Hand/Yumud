#pragma once

#include "../i2c_base.hpp"
#include "core/clock/clock.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{
class SoftI2c final: public I2cBase{
public:

    explicit SoftI2c(
        const Gpio & scl_pin,
        const Gpio & sda_pin
    ):
        scl_pin_(scl_pin),
        sda_pin_(sda_pin){;}

    SoftI2c(SoftI2c && other) = default;
    void init(const Config & cfg);
    Gpio & scl_pin() {return scl_pin_;}
    Gpio & sda_pin() {return sda_pin_;}


    HalResult borrow(const I2cSlaveAddrWithRw req);
    HalResult reset();
    HalResult write_byte(const uint32_t data) final;
    HalResult read_byte(uint8_t & data, const Ack ack) final;

    HalResult set_baudrate(const I2cBuadrate baudrate);
    void set_timeout(const Timeout timeout){timeout_ = timeout;}
    void discard_ack(const Enable en){discard_ack_ = (en == EN);}
    void lend(){
        this->trail();
        owner_.lend();
    }

private:
    Gpio scl_pin_;
    Gpio sda_pin_;

    Timeout timeout_ = Timeout(10);
    uint16_t delays_ = 10;
    bool discard_ack_ = false;

    PeripheralOwnershipTracker owner_ = {};


    __no_inline void delay_dur();

    HalResult wait_ack();
    HalResult lead(const I2cSlaveAddrWithRw req) final;
    void trail();
    HalResult unlock_bus();

};

}