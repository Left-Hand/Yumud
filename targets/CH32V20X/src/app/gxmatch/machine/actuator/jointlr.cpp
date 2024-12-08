#include "jointlr.hpp"
#include "hal/bus/uart/uarthw.hpp"



namespace gxm{
    
bool JointLR::reached(){
    return last_radian_ == expect_radian_;
}

void JointLR::setRadian(const real_t rad){
    expect_radian_ = rad;

    if(!inited_){
        last_radian_ = expect_radian_;
        inited_ = true;
    }

    output(last_radian_);
}

void JointLR::output(const real_t rad){
    // DEBUG_PRINTLN(rad);
    real_t out = [&](){
        if(inversed_){
            return (- rad + this->basis_radian_);
        }else{
            return (rad + this->basis_radian_);
        }
    }();

    servo_.setRadian(out);
    // DEBUG_PRINTLN(out);
}

void JointLR::tick(){
    if(reached() == false){
        // DEBUG_PRINTLN(last_radian_, expect_radian_);
        last_radian_ = STEP_TO(last_radian_, expect_radian_, max_rad_delta_);
        output(last_radian_);
    }
}

void JointLR::idle(){
    servo_.idle();
}
}