#include "chassis_actions.hpp"
#include "chassis_ctrl.hpp"

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
        // auto && pos_err = expect_pos - pos();
        // auto && rot_err = expect_rad - rot();
        // auto && delta = solver_.inverse(pos_err, rot_err);
        // wheels_.setDelta(delta);

    switch(ctrl_mode_){
        case CtrlMode::NONE:
            break;
        case CtrlMode::SHIFT:
            pos_ctrl_.update(expect_pos_, this->pos(), this->spd());
            break;
        case CtrlMode::SPIN:
            rot_ctrl_.update(expect_rot_, this->rot(), this->gyr());
            break;
    }
}


void ChassisModule::setCurrent(const Ray & ray){
    auto && curr = solver_.inverse(ray);
    wheels_.setCurrent(curr);
}

void ChassisModule::meta_rapid(const Ray & ray){
    TODO();
    // meta_rapid_shift(ray.org);
    // meta_rapid_spin(ray.rad);
}

void ChassisModule::meta_rapid_shift(const Vector2 & pos){
    expect_pos_ = pos;
    ctrl_mode_ = CtrlMode::SHIFT;
}

void ChassisModule::meta_rapid_spin(const real_t rad){
    expect_rot_ = rad;
    ctrl_mode_ = CtrlMode::SPIN;
}


Vector2 ChassisModule::pos(){
    return est_.pos();
}

Vector2 ChassisModule::spd(){
    return est_.spd();
}

real_t ChassisModule::rot(){
    return est_.rot();
}

Ray ChassisModule::gest(){
    return {this->pos(), this->rot()};
}


Ray ChassisModule::feedback(){
    return {0,0,0};
}

void ChassisModule::tick(){
    wheels_.update();
}

real_t ChassisModule::gyr(){
    return est_.gyr();
}

}

