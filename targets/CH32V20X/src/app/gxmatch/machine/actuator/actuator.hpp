#pragma once


#include "sys/math/real.hpp"

using namespace ymd;

namespace gxm{
class Actuator{
public:
    virtual bool reached() = 0;  
    virtual void tick() = 0;
};

class Axis:public Actuator{
public:
    virtual void setDistance(const real_t dist) = 0;
    virtual real_t getDistance() = 0;
};

class Joint:public Actuator{
public:
    virtual void setRadian(const real_t rad) = 0;
};

}