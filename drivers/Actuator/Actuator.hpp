#ifndef __ACTUATOR_HPP__

#define __ACTUATOR_HPP__

#include "hal/timer/pwm/pwm_channel.hpp"
#include "real.hpp"

class Actuator{
protected:
    bool enabled = false;
public:
    virtual void init() = 0;
    virtual void enable(const bool en = true) = 0;
};


class Servo:public Actuator{
public:

};


class ServoOpenLoop:public Servo{

};

class PwmBaseServo:public ServoOpenLoop{

};


class ServoCloseLoop:public Servo{
public:
    virtual real_t getPosition() = 0;
};

#endif