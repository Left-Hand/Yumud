#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{
class JointLR:public Joint{
    // Servo
public:
    bool reached() override;
    void setRad(const real_t dist) override;  
    void tick() override; 
};
}