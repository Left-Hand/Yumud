#pragma once

#include "sys/math/real.hpp"

class DistanceSensor{
public:
    virtual real_t getDistance();
    virtual void update();
};