#include "grab_module.hpp"
#include "actuator/jointlr.hpp"
#include "actuator/zaxis.hpp"
#include "nozzle/nozzle.hpp"

#include "../common/inc.hpp"

namespace gxm{

void GrabModule::goHome(){
    
}

void GrabModule::moveZ(const real_t pos){
    zaxis.setDistance(pos);
}

void GrabModule::moveXY(const Vector2 & pos){
    auto [a,b] = solver_.invrese(pos);
    joint_l.setRadian(a);
    joint_r.setRadian(b);
}


void GrabModule::moveTo(const Vector3 & pos){
    moveXY({pos.x, pos.y});
    moveZ(pos.z);
}

void GrabModule::pickUp(){
    claw.press();
    nozzle.press();
}

void GrabModule::putDown(){
    claw.release();
    nozzle.release();
}


void GrabModule::take(){
    
}

void GrabModule::give(){
    
}

bool GrabModule::done(){
    return true;
}

void GrabModule::begin(){
    
}

}