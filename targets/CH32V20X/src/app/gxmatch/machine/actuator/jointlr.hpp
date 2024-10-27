#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{
    
class JointLR:public Joint{
public:
    struct Config{
        real_t max_rad_delta;

        real_t left_basis_radian;
        real_t right_basis_radian;
    };
protected:
    const Config & config_;

    RadianServo & servo_;
    real_t last_radian_;
    real_t expect_radian_;

public:
    JointLR(const Config & config, RadianServo & servo):config_(config), servo_(servo){}
    bool reached() override;
    void setRadian(const real_t dist) override; 
    real_t getRadian(){return std::bit_cast<real_t>(last_radian_);}
    void tick() override; 
};

}