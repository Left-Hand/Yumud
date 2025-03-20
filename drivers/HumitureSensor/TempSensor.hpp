#pragma once

#include "drivers/device_defs.h"
#include "core/math/real.hpp"

namespace ymd::drivers{
class TempSensor{
public:
    virtual real_t getTemp() = 0;
};

}