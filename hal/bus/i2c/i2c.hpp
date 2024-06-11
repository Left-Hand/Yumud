#ifndef __I2C_HPP__

#define __I2C_HPP__

#include "../serbus.hpp"

class I2c:public SerBus{
protected:
    uint32_t timeout = 10;
    virtual void reset() = 0;
    virtual Error start(const uint8_t & _address) = 0;
    virtual void stop() = 0;
    virtual void lock_avoid() = 0;
public:
    void setTimeout(const uint32_t _timeout){timeout = _timeout;}
};

class I2cHw : public I2c{
public:
    virtual void enableHwTimeout(const bool en = true) = 0;
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