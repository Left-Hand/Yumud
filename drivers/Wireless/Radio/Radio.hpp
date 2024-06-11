#ifndef __RADIO_HPP__

#define __RADIO_HPP__

#include "src/gpio/gpio.hpp"
class Radio{
public:
protected:
    uint16_t channel;
public:
    virtual void setChannel(const uint16_t _channel) = 0;
};


#endif