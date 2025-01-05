#pragma once

#include "i2c.hpp"

namespace ymd::hal{

class I2cHw: public I2c{
private:
    void enableRcc(const bool enable = true);
    Error lead(const uint8_t _address) override final;
    void trail() override final;

protected:
    I2C_TypeDef * instance;
    static GpioConcept & getScl(const I2C_TypeDef *);
    static GpioConcept & getSda(const I2C_TypeDef *);
public:

    I2cHw(I2C_TypeDef * _instance):
            I2c(getScl(_instance), getSda(_instance)),
            instance(_instance){;}
    

    Error write(const uint32_t data) override final;
    Error read(uint32_t & data, const bool toAck = true) override final;
    void init(const uint32_t baudRate) override ;
    void reset();
    bool locked();
    void unlock_bus();
    void setBaudRate(const uint32_t baudRate) override{;}//TODO
    void enableHwTimeout(const bool en = true);
};

#ifdef ENABLE_I2C1
inline ymd::I2cHw i2c1{I2C1};
#endif

#ifdef ENABLE_I2C2
inline ymd::I2cHw i2c2{I2C2};
#endif

}