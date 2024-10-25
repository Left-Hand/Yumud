#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{
    
class JointLR:public Joint{
    RadianServo & servo_;
    real_t last_radian_;

    struct Refs{
        std::reference_wrapper<RadianServo> _servo;
        
    };
public:
    JointLR(const Refs & refs):servo_(refs._servo){}
    bool reached() override;
    void setRadian(const real_t dist) override;  
    void tick() override; 
};

}