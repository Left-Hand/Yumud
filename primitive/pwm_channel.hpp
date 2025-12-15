#pragma once
#include "core/math/real.hpp"

namespace ymd::hal{
class PwmIntf{
public:
    virtual void set_dutycycle(const uq16 dutycycle) = 0;
};

// template <typename T>
// class CountableIntf{
// public:
//     virtual volatile T & cnt() = 0;
//     virtual volatile T & cvr() = 0;
//     virtual volatile T & arr() = 0;
// };
}