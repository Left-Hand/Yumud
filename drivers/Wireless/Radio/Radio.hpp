#pragma once

#include "hal/gpio/gpio.hpp"


namespace ymd::drivers{

class Radio{
public:
protected:
    uint16_t channel;
public:
    virtual void setChannel(const uint16_t _channel) = 0;
};

}