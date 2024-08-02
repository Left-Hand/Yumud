#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "../types/real.hpp"

class PwmChannel{
public:
    virtual void init() = 0;

    virtual PwmChannel & operator = (const real_t duty) = 0;
};

class Countable{
public:
    virtual volatile uint16_t & cnt() = 0;
    virtual volatile uint16_t & cvr() = 0;
    virtual volatile uint16_t & arr() = 0;
};
#endif