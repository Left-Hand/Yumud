#pragma once


#include "core/math/real.hpp"

namespace ymd::drivers{
class DistanceSensor{
public:
    virtual real_t getDistance();
    virtual void update();
};

}