#include "scara.hpp"

namespace gxm{

void Scara::goHome(){
    joint_l.setRadian(real_t(PI/2) + config_.joint_config.left_basis_radian);
    joint_r.setRadian(real_t(PI/2) + config_.joint_config.right_basis_radian);
}

void Scara::moveXY(const Vector2 & pos){
    auto [a,b] = solver_.invrese(pos);
    joint_l.setRadian(a + config_.joint_config.left_basis_radian);
    joint_r.setRadian(b + config_.joint_config.right_basis_radian);
}


void Scara::pickUp(){
    claw.press();
    nozzle.press();
}

void Scara::putDown(){
    claw.release();
    nozzle.release();
}

bool Scara::reached(){
    return joint_l.reached() and joint_r.reached();
}


}