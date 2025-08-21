#pragma once
#include "core/math/real.hpp"

namespace ymd::hal{
class PwmIntf{
public:
    virtual void set_dutycycle(const real_t duty) = 0;
    virtual PwmIntf & operator = (const real_t duty) final{
        this->set_dutycycle(duty);
        return *this;
    }
};

template <typename T>
class Countable{
public:
    virtual volatile T & cnt() = 0;
    virtual volatile T & cvr() = 0;
    virtual volatile T & arr() = 0;
};
}