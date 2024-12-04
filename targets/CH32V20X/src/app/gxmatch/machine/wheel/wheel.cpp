#include "wheel.hpp"

namespace gxm{

// void Wheel::setSpeed(const real_t spd){
//     setMotorSpeed(World2Motor(spd));
// }

/// @brief 
/// @param pos 
void Wheel::setPosition(const real_t pos){
    last_targ_position = World2Motor(pos);
    setMotorPosition(last_targ_position);
}

void Wheel::forwardPosition(const real_t step){
    last_targ_position += World2Motor(step);
    setMotorPosition(last_targ_position);
}

void Wheel::setCurrent(const real_t curr){
    motor_.setTargetCurrent(curr);
}

// real_t Wheel::getSpeed(){
//     return Motor2World(getMotorSpeed());
// }
real_t Wheel::getPosition(){
    return Motor2World(getMotorPosition());
}
    
}