#include "wheel.hpp"

namespace gxm{

void Wheel::setSpeed(const real_t spd){
    setMotorSpeed(World2Motor(spd));
}

void Wheel::setPosition(const real_t pos){
    setMotorPosition(World2Motor(pos));
}

void Wheel::forwardPosition(const real_t step){
    setMotorPosition(getMotorPosition() + World2Motor(step));
}

real_t Wheel::getSpeed(){
    return Motor2World(getMotorSpeed());
}
real_t Wheel::getPosition(){
    return Motor2World(getMotorPosition());
}
    
}