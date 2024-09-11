#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "../types/real.hpp"

class PwmChannel{
public:
    virtual void init() = 0;

    virtual PwmChannel & operator = (const real_t duty) = 0;
};

template <typename T>
class Countable{
public:
    virtual volatile T & cnt() = 0;
    virtual volatile T & cvr() = 0;
    virtual volatile T & arr() = 0;
};
#endif