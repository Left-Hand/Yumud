#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/Servo/PwmServo/pwm_servo.hpp"

namespace gxm{
class JointLR:public Joint{
    
public:
    bool reached() override;
    void setRad(const real_t dist) override;  
    void tick() override; 
};
}