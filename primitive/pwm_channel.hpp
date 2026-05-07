#pragma once
#include "core/math/real.hpp"

namespace ymd::hal{
class PwmIntf{
public:
    virtual void set_dutycycle(const uq16 dutycycle) = 0;
};

}