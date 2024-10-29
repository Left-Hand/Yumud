#include "claw.hpp"

#include "drivers/Actuator/servo/servo.hpp"

namespace gxm{
    

void Claw::press(){
    servo_.setRadian(config_.press_radian);
}


void Claw::release(){
    servo_.setRadian(config_.release_radian);
}

}