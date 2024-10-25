#include "chassis_module.hpp"

namespace gxm{


void ChassisModule::positionTrim(const Vector2 & trim){
    
}

void ChassisModule::rotationTrim(const real_t raderr){

}

void ChassisModule::forwardMove(const Vector2 & vel, const real_t spinrate){
    auto [s0, s1, s2, s3] = solver_.inverse(vel, spinrate);
    wheels_[0].get().setSpeed(s0);
    wheels_[1].get().setSpeed(s1);
    wheels_[2].get().setSpeed(s2);
    wheels_[3].get().setSpeed(s3);
}

void ChassisModule::calibrateRotation(const real_t rad){
    
}

void ChassisModule::calibratePosition(const Vector2 & pos){
    
}

void ChassisModule::tick(){

}
    
}