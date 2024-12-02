#include "scara.hpp"

namespace gxm{

void Scara::goHome(){
    joint_l.setRadian(real_t(PI/2));
    joint_r.setRadian(real_t(PI/2));
}

void Scara::moveXY(const Vector2 & pos){
    auto [a,b] = solver_.inverse(pos);
    joint_l.setRadian(a);
    joint_r.setRadian(b);
}


Vector2 Scara::getPos(){
    return solver_.forward(joint_l.getRadian(), joint_r.getRadian());
}

void Scara::press(){
    claw.press();
    nozzle.press();
}

void Scara::release(){
    claw.release();
    nozzle.release();
}

bool Scara::reached(){
    return joint_l.reached() and joint_r.reached();
}


}