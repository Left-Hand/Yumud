#pragma once

#include "i2c.hpp"
#include "core/clock/clock.hpp"

namespace ymd::hal{
class I2cSw final: public I2c{
private:
    volatile int8_t occupied_ = -1;

    uint16_t delays_ = 10;

    __no_inline void delay_dur();

    hal::BusError wait_ack();
    hal::BusError lead(const uint8_t address) final override;
    void trail() override;
protected :
    hal::BusError reset();
    hal::BusError unlock_bus();
public:

    I2cSw(hal::Gpio & scl,hal::Gpio & sda):I2c(scl, sda){;}
    hal::BusError write(const uint32_t data) final override;
    hal::BusError read(uint32_t & data, const Ack ack) final override;
    void init(const uint32_t baudrate);
    void set_baudrate(const uint32_t baudrate);
};

}