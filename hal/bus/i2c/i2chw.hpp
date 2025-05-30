#pragma once

#include "i2c.hpp"
#include "core/sdk.hpp"

namespace ymd::hal{

class I2cHw final: public I2c{
private:
    void enable_rcc(const Enable enable = EN);
    hal::HalResult lead(const LockRequest req) final;
    void trail() final;

protected:
    I2C_TypeDef * instance;
    static hal::Gpio & get_scl(const I2C_TypeDef *);
    static hal::Gpio & get_sda(const I2C_TypeDef *);
public:

    I2cHw(I2C_TypeDef * _instance):
            I2c(get_scl(_instance), get_sda(_instance)),
            instance(_instance){;}
    

    hal::HalResult write(const uint32_t data) final;
    hal::HalResult read(uint32_t & data, const Ack ack) final;
    void init(const uint32_t baudrate);
    void reset();
    bool locked();
    hal::HalResult unlock_bus();
    void enable_hw_timeout(const Enable en = EN);
};


#ifdef ENABLE_I2C1
extern I2cHw i2c1;
#endif

#ifdef ENABLE_I2C2
extern I2cHw i2c2;
#endif

}