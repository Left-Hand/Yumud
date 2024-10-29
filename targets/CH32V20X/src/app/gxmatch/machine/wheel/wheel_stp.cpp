#include "wheel_stp.hpp"
#include "robots/foc/focmotor.hpp"

namespace gxm{

void WheelStepper::setMotorSpeed(const real_t spd) {
    motor_.setTargetSpeed(spd);
}


void WheelStepper::setMotorPosition(const real_t pos) {
    motor_.setTargetPosition(pos);
}


real_t WheelStepper::getMotorSpeed() {
    return motor_.getSpeed();
}


real_t WheelStepper::getMotorPosition() {
    return motor_.getPosition();
}

}