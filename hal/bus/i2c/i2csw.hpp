#pragma once

#include "i2c.hpp"
#include "sys/clock/clock.h"

namespace ymd::hal{
class I2cSw: public I2c{
private:
    volatile int8_t occupied_ = -1;

    uint16_t delays_ = 10;

    __no_inline void delayDur();

    BusError wait_ack();
    BusError lead(const uint8_t address) final override;
    void trail() override;
protected :
    BusError reset();
    BusError unlock_bus();
public:

    I2cSw(GpioIntf & scl,GpioIntf & sda):I2c(scl, sda){;}
    BusError write(const uint32_t data) final override;
    BusError read(uint32_t & data, const Ack ack) final override;
    void init(const uint32_t baudRate);
    void setBaudRate(const uint32_t baudRate);
};

}