#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{
    
class JointLR:public Joint{
public:
    struct Config{
        real_t max_rad_delta;
        Range left_radian_clamp;
        Range right_radian_clamp;
    };
protected:
    RadianServo & servo_;
    real_t last_radian_;
    real_t expect_radian_;
    
    const Config & config_;


public:
    JointLR(RadianServo & servo, const Config & config):servo_(servo), config_(config){}
    bool reached() override;
    void setRadian(const real_t dist) override; 
    real_t getRadian(){return std::bit_cast<real_t>(last_radian_);}
    void tick() override; 
};

}