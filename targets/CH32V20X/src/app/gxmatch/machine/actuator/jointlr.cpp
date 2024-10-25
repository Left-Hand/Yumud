#include "jointlr.hpp"
#include "hal/bus/uart/uarthw.hpp"

namespace gxm{
    

bool JointLR::reached(){
    return last_radian_ == expect_radian_;
}

void JointLR::setRadian(const real_t rad){
    expect_radian_ = rad;

}

void JointLR::tick(){
    last_radian_ = STEP_TO(last_radian_, expect_radian_, config_.max_rad_delta);
    servo_.setRadian(last_radian_);

    // uart2.println(last_radian_, expect_radian_, config_.max_rad_delta);
}

}