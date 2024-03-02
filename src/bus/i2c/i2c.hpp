#ifndef __I2C_HPP__

#define __I2C_HPP__

#include "../serbus.hpp"
#include "src/comm_inc.h"


class I2c:public SerBus{
private:
    uint32_t timeout = 0;
protected:
    virtual Error start(const uint8_t & _address) = 0;
    virtual void stop() = 0;
    void set_timeout(const uint32_t _timeout){timeout = _timeout;}
};

#endif