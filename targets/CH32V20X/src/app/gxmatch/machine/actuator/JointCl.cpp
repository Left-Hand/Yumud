#include "JointCl.hpp"

namespace gxm{
    
bool JointCl::reached(){
    return last_radian_ == expect_radian_;
}

void JointCl::setRadian(const real_t rad){
    expect_radian_ = rad;
}

void JointCl::tick(){
    last_radian_ = STEP_TO(last_radian_, expect_radian_, config_.max_rad_delta);
    servo_.setRadian(last_radian_);
}

}