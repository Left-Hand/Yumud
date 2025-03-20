#pragma once

#include "drivers/device_defs.h"
#include "types/range/range.hpp"

namespace ymd::drivers{
class ActuatorIntf{
// protected:
//     bool enabled = false;
public:
    // virtual void init() = 0;
    virtual void enable(const bool en) = 0;
    virtual ~ActuatorIntf() = default;
};
}