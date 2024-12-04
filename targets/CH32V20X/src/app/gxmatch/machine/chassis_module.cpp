#include "chassis_actions.hpp"

namespace gxm{


void ChassisModule::positionTrim(const Vector2 & trim){
    wheels_.setDelta(solver_.inverse(trim, 0));
}

void ChassisModule::rotationTrim(const real_t raderr){
    wheels_.setDelta(solver_.inverse({0,0}, raderr));
}

void ChassisModule::forwardMove(const Vector2 & vel, const real_t spinrate){
    // wheels_.setSpeed(solver_.inverse(vel, spinrate));
}

void ChassisModule::calibrateRotation(const real_t rad){
    
}

void ChassisModule::calibratePosition(const Vector2 & pos){
    
}

void ChassisModule::test(){

}
    

bool ChassisModule::arrived(){
    return false;
}

void ChassisModule::closeloop(){
    auto && pos_err = expect_pos - pos();
    auto && rad_err = expect_rad - rad();
    auto && delta = solver_.inverse(pos_err, rad_err);
    wheels_.setDelta(delta);
}


void ChassisModule::meta_rapid(const Ray & ray){
    meta_rapid_shift(ray.org);
    meta_rapid_spin(ray.rad);
}

void ChassisModule::meta_rapid_shift(const Vector2 & pos){
    expect_pos = pos;
}

void ChassisModule::meta_rapid_spin(const real_t rad){
    expect_rad = rad;
}


Vector2 ChassisModule::pos(){
    TODO();

    return {0,0};
}

real_t ChassisModule::rad(){
    TODO();

    return 0;
}

Ray ChassisModule::gest(){
    return {this->pos(), this->rad()};
}


Ray ChassisModule::feedback(){
    return {0,0,0};
}

void ChassisModule::tick(){
    wheels_.update();
}

}

