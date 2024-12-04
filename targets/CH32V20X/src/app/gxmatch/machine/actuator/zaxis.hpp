#pragma once

#include "actuator.hpp"

namespace gxm{
class ZAxis:public Axis{
    
public:
    struct Config{
        // real_t max_height;
        // real_t tray_height;
        // real_t free_height;
        // real_t ground_height;
        
        // real_t meter_to_turns_scaler;
        // real_t reached_threshold;
        // real_t basis_radian;
    };

private:
    const Config & config_;
public:
    ZAxis(const Config & config):config_(config){}
    DELETE_COPY_AND_MOVE(ZAxis)

    virtual void softHome() = 0;
};


}