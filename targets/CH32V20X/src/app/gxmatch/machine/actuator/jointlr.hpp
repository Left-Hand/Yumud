#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{
using namespace ymd::drivers;
    
class JointLR:public Joint{
public:
    // struct Config{
    //     real_t max_rad_delta;

    //     real_t z_basis_radian;
    //     real_t left_basis_radian;
    //     real_t right_basis_radian;
    // };
protected:
    // const Config & config_;
    const real_t max_rad_delta_;
    const real_t basis_radian_;
    bool inversed_ = false;
    bool inited_ = false;

    RadianServo & servo_;
    real_t last_radian_;
    real_t expect_radian_;

    void output(const real_t rad);
public:
    JointLR(const real_t max_rad_delta, const real_t basis_radian, RadianServo & servo):
        max_rad_delta_(max_rad_delta),
        basis_radian_(basis_radian),
        servo_(servo){}
    
    DELETE_COPY_AND_MOVE(JointLR)
    
    bool reached() override;
    void setRadian(const real_t dist) override; 
    real_t getRadian(){return std::bit_cast<real_t>(last_radian_);}
    void tick() override;
    void inverse(const bool en = true){
        inversed_ = en;
    }
};

}