#pragma once

#include "core/io/regs.hpp"
#include "core/math/real.hpp"

namespace ymd::drivers{
class TempSensor{
public:
    virtual real_t getTemp() = 0;
};

}