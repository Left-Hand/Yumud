#pragma once

#include "../drivers/device_defs.h"

namespace ymd::drivers{
class DistanceSensor{
public:
    virtual real_t getDistance();
    virtual void update();
};

}