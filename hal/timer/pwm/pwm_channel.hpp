#pragma once
#include "sys/math/real.hpp"

namespace ymd{
class PwmChannelIntf{
public:
    virtual PwmChannelIntf & operator = (const real_t duty) = 0;
};

template <typename T>
class Countable{
public:
    virtual volatile T & cnt() = 0;
    virtual volatile T & cvr() = 0;
    virtual volatile T & arr() = 0;
};
}