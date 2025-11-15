#include "scara.hpp"

namespace ymd::robots{

void Scara::goHome(){
    joint_l.set_radian(real_t(PI/2));
    joint_r.set_radian(real_t(PI/2));
}

void Scara::moveXY(const Vec2<iq16> & pos){
    auto [a,b] = solver_.inverse(pos);
    joint_l.set_radian(a);
    joint_r.set_radian(b);
}


Vec2<iq16> Scara::getPos(){
    return solver_.forward(joint_l.get_radian(), joint_r.get_radian());
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
    // DEBUG_PRINTLN(joint_l.reached(), joint_r.reached());
    return joint_l.reached() and joint_r.reached();
}


bool Scara::caught(){
    return true;
}


void Scara::idle(){
    joint_l.idle();
    joint_r.idle();
}
}