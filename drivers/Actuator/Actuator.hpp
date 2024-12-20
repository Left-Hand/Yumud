#pragma once

#include "drivers/device_defs.h"

namespace ymd::drivers{
class Actuator{
protected:
    bool enabled = false;
public:
    virtual void init() = 0;
    virtual void enable(const bool en = true) = 0;
};
}