#include "grab_module.hpp"
#include "actuator/jointlr.hpp"
#include "actuator/zaxis.hpp"
#include "nozzle/nozzle.hpp"

#include "../common/inc.hpp"

namespace gxm{

void GrabModule::goHome(){
    // joint_l.setRadian(config_.joint_config.right_basis_radian);
    // joint_r.setRadian(real_t(PI) + config_.joint_config.right_basis_radian);
    joint_l.setRadian(real_t(PI/2) + config_.joint_config.left_basis_radian);
    joint_r.setRadian(real_t(PI/2) + config_.joint_config.right_basis_radian);
}

void GrabModule::moveZ(const real_t pos){
    zaxis.setDistance(pos);
}

void GrabModule::moveXY(const Vector2 & pos){
    auto [a,b] = solver_.invrese(pos);
    // uart2.println(a,b);
    joint_l.setRadian(a + config_.joint_config.left_basis_radian);
    joint_r.setRadian(b + config_.joint_config.right_basis_radian);
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
    return zaxis.reached() and joint_l.reached() and joint_r.reached();
}

void GrabModule::begin(){
    
}

}