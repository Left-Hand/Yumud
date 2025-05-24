#pragma once

#include "core/io/regs.hpp"
#include "types/regions/range2/range2.hpp"

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