#pragma once

#include "actuator.hpp"

namespace gxm{
class ZAxis:public Axis{
    
public:
    struct Config{
        uint max_height_mm;
        uint tray_height_mm;
        uint free_height_mm;
        uint ground_height_mm;
        real_t meter_to_turns_scaler;
        real_t reached_threshold;

        real_t kp;
        real_t kd;
    };

protected:
    const Config & config_;
public:
    ZAxis(const Config & config):config_(config){}
    DELETE_COPY_AND_MOVE(ZAxis)

    virtual void softHome() = 0;
};


}