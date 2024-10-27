#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{

class Claw{
public:
    struct Config{
        
    };
protected:
    const Config & config_;
public:
    Claw(const Config & config):config_(config){;}

    void press();
    void release();
};
    
}