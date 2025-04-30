#include "wheel.hpp"

namespace ymd::robots{


void Wheel::setPosition(const real_t pos){
    last_targ_position = World2Motor(pos);
    setMotorPosition(INVERSE_IF(inversed_, last_targ_position));
}

void Wheel::updatePosition(){
    motor_.getPosition();
}

void Wheel::freeze(){
    motor_.freeze();
}

real_t Wheel::readPosition(){
    return INVERSE_IF(inversed_, Motor2World(motor_.readPosition()));
}
    // void updatePosition(const real_t pos); 
    // real_t readPosition(); 

void Wheel::setSpeed(const real_t spd){
    auto _spd = World2Motor(spd);
    setMotorSpeed(INVERSE_IF(inversed_, _spd));
}

void Wheel::setVector(const real_t pos){
    auto _pos = World2Motor(pos);
    motor_.setTargetVector(INVERSE_IF(inversed_, _pos));
}

void Wheel::forwardPosition(const real_t step){
    last_targ_position += World2Motor(step);
    setMotorPosition(INVERSE_IF(inversed_, last_targ_position));
}

void Wheel::setCurrent(const real_t curr){
    motor_.setTargetCurrent(INVERSE_IF(inversed_, curr));
}

// real_t Wheel::getPosition(){
//     return INVERSE_IF(inversed_, Motor2World(getMotorPosition()));
// }

void Wheel::setMotorPosition(const real_t pos){
    motor_.setTargetPosition(pos);
}

void Wheel::setMotorSpeed(const real_t spd){
    motor_.setTargetSpeed(spd);
}
real_t Wheel::getMotorPosition(){
    return motor_.getPosition();
}


// void Wheel::init(){
//     motor_.reset();
//     motor_.
// }

}