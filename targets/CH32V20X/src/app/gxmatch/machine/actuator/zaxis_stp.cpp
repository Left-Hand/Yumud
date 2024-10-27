#include "zaxis_stp.hpp"
#include "robots/foc/focmotor.hpp"

namespace gxm{

bool ZAxisStepper::reached(){
    return ABS(motor_.getSpeed()) <= real_t(0.02);
}

void ZAxisStepper::setDistance(const real_t dist){
    motor_.setTargetPosition(dist * 20);
}

void ZAxisStepper::tick(){
    
}

void ZAxisStepper::softHome(){
    motor_.setTargetCurrent(real_t(-0.4));
}


}