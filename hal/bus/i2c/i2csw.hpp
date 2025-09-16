#pragma once

#include "i2c.hpp"
#include "core/clock/clock.hpp"


namespace ymd::hal{
class I2cSw final: public I2c{
private:

    uint16_t delays_ = 10;

    __no_inline void delay_dur();

    hal::HalResult wait_ack();
    hal::HalResult lead(const hal::LockRequest req) final;
    void trail();

protected :
    hal::HalResult unlock_bus();
public:

    I2cSw(
        Some<hal::Gpio *> scl,
        Some<hal::Gpio *> sda
    ):
        I2c(scl, sda){;}
    I2cSw(I2cSw && other) = default;
    
    hal::HalResult reset();
    hal::HalResult write(const uint32_t data) final;
    hal::HalResult read(uint32_t & data, const Ack ack) final;
    void init(const Config & cfg);
    hal::HalResult set_baudrate(const uint32_t baudrate);
};

}