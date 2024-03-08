#ifndef __ACTUATOR_HPP__

#define __ACTUATOR_HPP__

#include "src/timer/pwm_channel.hpp"
#include "real.hpp"

class Actuator{
public:
    virtual void init() = 0;
    virtual void enable(const bool en = true) = 0;
    virtual void setDuty(const real_t & duty) = 0;
};


class Servo:public Actuator{
public:

};

class ServoOpenLoop:public Servo{

};

class ServoCloseLoop:public Servo{
public:
    virtual const real_t getAngle();
};

#endif