#pragma once


#include "sys/math/real.hpp"

namespace gxm{
class Actuator{
    virtual bool reached() = 0;  
    virtual void tick() = 0;
};

class Axis:public Actuator{
    virtual void setDistance(const real_t dist) = 0;
};

class Joint:public Actuator{
    virtual void setRadian(const real_t rad) = 0;
};

}