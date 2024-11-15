#pragma once

#include "sys/math/real.hpp"



namespace ymd::drivers{
class DistanceSensor{
public:
    virtual real_t getDistance();
    virtual void update();
};

}