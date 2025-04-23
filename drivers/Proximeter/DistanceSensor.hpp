#pragma once


#include "core/math/real.hpp"

namespace ymd::drivers{
class DistanceSensor{
public:
    virtual real_t get_distance();
    virtual void update();
};

}