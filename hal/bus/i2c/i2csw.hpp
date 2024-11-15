#pragma once

#include "i2c.hpp"
#include "sys/clock/clock.h"

namespace ymd{
class I2cSw: public I2c{
private:
    volatile int8_t occupied = -1;

    uint16_t delays = 10;

    __fast_inline void delayDur(){
        delayMicroseconds(delays);
    }

    Error wait_ack();
    Error lead(const uint8_t _address) override;
    void trail() override;
protected :
    void reset() override {};
    void unlock_bus() override {};
public:

    I2cSw(GpioConcept & scl,GpioConcept & sda):I2c(scl, sda){;}
    Error write(const uint32_t data) final override;
    Error read(uint32_t & data, bool toAck = true) final override;
    void init(const uint32_t baudRate);
    void setBaudRate(const uint32_t baudRate) final override;
};

}