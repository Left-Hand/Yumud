#pragma once

#include "actuator.hpp"

namespace gxm{
class ZAxis:public Axis{
    
public:
    struct Config{
        real_t max_height_mm;
        uint tray_height_mm;
        uint free_height_mm;
        uint ground_height_mm;
    };

protected:
    const Config & config_;
public:
    ZAxis(const Config & config):config_(config){}

    bool reached() override;
    void setDistance(const real_t dist) override;  
    void tick() override;

    void softHome();
};


}