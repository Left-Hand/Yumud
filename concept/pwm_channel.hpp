#pragma once
#include "core/math/real.hpp"

namespace ymd::hal{
class PwmIntf{
public:
    void set_duty(const real_t duty){
        *this = duty;
    }
    virtual PwmIntf & operator = (const real_t duty) = 0;
};

template <typename T>
class Countable{
public:
    virtual volatile T & cnt() = 0;
    virtual volatile T & cvr() = 0;
    virtual volatile T & arr() = 0;
};
}