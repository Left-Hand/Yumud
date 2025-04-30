#include "claw.hpp"

#include "drivers/Actuator/servo/servo.hpp"

namespace ymd::robots{
    

void Claw::press(){
    servo_.set_radian(config_.press_radian);
}


void Claw::release(){
    servo_.set_radian(config_.release_radian);
}

}