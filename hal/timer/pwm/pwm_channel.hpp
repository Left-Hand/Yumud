#ifndef __PWMCHANNEL_HPP__

#define __PWMCHANNEL_HPP__

#include "types/real.hpp"

class PwmChannel{
protected:
    real_t min_value = real_t(0);
    real_t max_value = real_t(1);
public:
    void setClamp(const real_t & abs_max_value){
        min_value = real_t(0);
        max_value = abs(abs_max_value);
    }
    void setClamp(const real_t & _min_value, const real_t & _max_value){
        min_value = MAX(_min_value, real_t(0));
        max_value = MIN(_max_value, real_t(1));
    }

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