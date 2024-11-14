#include "chassis_module.hpp"

namespace gxm{


void ChassisModule::positionTrim(const Vector2 & trim){
    wheels_.forwardPosition(solver_.inverse(trim, 0));
}

void ChassisModule::rotationTrim(const real_t raderr){
    wheels_.forwardPosition(solver_.inverse({0,0}, raderr));
}

void ChassisModule::forwardMove(const Vector2 & vel, const real_t spinrate){
    wheels_.setSpeed(solver_.inverse(vel, spinrate));
}

void ChassisModule::calibrateRotation(const real_t rad){
    
}

void ChassisModule::calibratePosition(const Vector2 & pos){
    
}

void ChassisModule::tick(){

}
    
}

