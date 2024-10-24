#pragma once

#include "actuator.hpp"

namespace gxm{
class ZAxis:public Axis{
    
public:
    bool reached() override;
    void setDistance(const real_t dist) override;  
    void tick() override;

    void softHome();
};


}