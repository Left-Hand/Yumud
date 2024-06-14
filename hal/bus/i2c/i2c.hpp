#ifndef __I2C_HPP__

#define __I2C_HPP__

#include "hal/bus/bus.hpp"

class I2c:public ProtocolBus{
protected:
    uint32_t timeout = 10;
    virtual void reset() = 0;

    virtual void unlock() = 0;
public:
    void setTimeout(const uint32_t _timeout){timeout = _timeout;}
    virtual void init(const uint32_t baudrate) = 0;
};

#define I2C_WAIT_COND(cond, timeout)\
{\
    uint32_t begin_time = micros();\
    while(!cond){\
        if(micros() - begin_time > timeout){\
            end_use();\
            return ErrorType::NACK;\
        }\
    }\
}\

#endif