#pragma once

#include "i2c.hpp"

namespace ymd::hal{

class I2cHw: public I2c{
private:
    void enableRcc(const bool enable = true);
    BusError lead(const uint8_t _address) override final;
    void trail() override final;

protected:
    I2C_TypeDef * instance;
    static hal::GpioIntf & getScl(const I2C_TypeDef *);
    static hal::GpioIntf & getSda(const I2C_TypeDef *);
public:

    I2cHw(I2C_TypeDef * _instance):
            I2c(getScl(_instance), getSda(_instance)),
            instance(_instance){;}
    

    BusError write(const uint32_t data) override final;
    BusError read(uint32_t & data, const Ack ack) override final;
    void init(const uint32_t baudRate);
    void reset();
    bool locked();
    void unlock_bus();
    void enableHwTimeout(const bool en = true);
};

#ifdef ENABLE_I2C1
extern I2cHw i2c1;
#endif

#ifdef ENABLE_I2C2
extern I2cHw i2c2;
#endif

}