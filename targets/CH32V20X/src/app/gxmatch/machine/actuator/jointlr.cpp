#include "jointlr.hpp"

namespace gxm{
    

bool JointLR::reached(){
    return servo.getRadian() == last_radian_;
}

void JointLR::setRadian(const real_t rad){
    servo.setRadian(rad);
    last_radian_ = rad;
}

void JointLR::tick(){
    
}

}