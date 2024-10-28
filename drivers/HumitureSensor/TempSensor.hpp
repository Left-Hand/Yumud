#pragma once

#include "drivers/device_defs.h"

class TempSensor{
public:
    virtual void init() = 0;
    virtual void update() = 0;
    virtual real_t getTemp() = 0;
};

