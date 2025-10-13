#pragma once

#include "i2c.hpp"
#include "core/clock/clock.hpp"


namespace ymd::hal{
class I2cSw final: public I2c{
public:

    I2cSw(
        Some<hal::Gpio *> scl,
        Some<hal::Gpio *> sda
    ):
        scl_(scl.deref()),
        sda_(sda.deref()){;}
    I2cSw(I2cSw && other) = default;

    hal::Gpio & scl() {return scl_;}
    hal::Gpio & sda() {return sda_;}

    hal::HalResult reset();
    hal::HalResult write(const uint32_t data) final;
    hal::HalResult read(uint32_t & data, const Ack ack) final;
    void init(const Config & cfg);
    hal::HalResult set_baudrate(const uint32_t baudrate);
private:
    hal::Gpio & scl_;
    hal::Gpio & sda_;

    uint16_t delays_ = 10;

    __no_inline void delay_dur();

    hal::HalResult wait_ack();
    hal::HalResult lead(const hal::I2cSlaveAddrWithRw req) final;
    void trail();
    hal::HalResult unlock_bus();

};

}