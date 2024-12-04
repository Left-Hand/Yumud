#include "wheel.hpp"

namespace gxm{


void Wheel::setPosition(const real_t pos){
    last_targ_position = World2Motor(pos);
    setMotorPosition(last_targ_position);
}

void Wheel::forwardPosition(const real_t step){
    last_targ_position += World2Motor(step);
    setMotorPosition(last_targ_position);
}

void Wheel::setCurrent(const real_t curr){
    motor_.setTargetCurrent(INVERSE_IF(inversed_, curr));
}

real_t Wheel::getPosition(){
    return INVERSE_IF(inversed_, Motor2World(getMotorPosition()));
}

void Wheel::setMotorPosition(const real_t pos){
    motor_.setTargetPosition(pos);
}
real_t Wheel::getMotorPosition(){
    return motor_.getPosition();
}


}