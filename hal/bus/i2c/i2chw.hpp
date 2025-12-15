#pragma once

#include "i2c.hpp"
#include "hal/gpio/gpio.hpp"

namespace ymd::hal{


class I2cHw final: public I2c{
public:

    I2cHw(void * inst);

    [[nodiscard]] HalResult write(const uint32_t data);
    [[nodiscard]] HalResult read(uint8_t & data, const Ack ack);
    void init(const uint32_t baudrate);
    void reset();
    bool locked();
    [[nodiscard]] HalResult unlock_bus();
    void enable_hw_timeout(const Enable en);

    hal::Gpio & scl() {return scl_;}
    hal::Gpio & sda() {return scl_;}
private:
    void * inst_;
    hal::Gpio scl_;
    hal::Gpio sda_;

    void enable_rcc(const Enable en);
    [[nodiscard]] HalResult lead(const I2cSlaveAddrWithRw req);
    void trail();
};


#ifdef I2C1_PRESENT
extern I2cHw i2c1;
#endif

#ifdef I2C2_PRESENT
extern I2cHw i2c2;
#endif

}