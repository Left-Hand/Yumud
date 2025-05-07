#include "zaxis_cross.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"


namespace ymd::robots{

bool ZAxisCross::reached(){
    // return motor_.reached();
    return true;
}

real_t ZAxisCross::getDistance(){
    return solver_.forward(motor_.get_radian());
}

void ZAxisCross::setDistance(const real_t dist){
    motor_.set_radian(solver_.inverse(dist));
}

void ZAxisCross::tick(){
    motor_.tick();
}

void ZAxisCross::softHome(){
    this->setDistance(0.15_r);
}


}