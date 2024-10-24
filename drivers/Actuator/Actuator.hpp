#ifndef __ACTUATOR_HPP__

#define __ACTUATOR_HPP__

#include "../hal/timer/pwm/pwm_channel.hpp"
#include "sys/math/real.hpp"

class Actuator{
protected:
    bool enabled = false;
public:
    virtual void init() = 0;
    virtual void enable(const bool en = true) = 0;
};


#endif