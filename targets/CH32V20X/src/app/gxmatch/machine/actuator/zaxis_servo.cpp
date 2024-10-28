#include "zaxis_servo.hpp"

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"
#include "drivers/Proximeter/VL53L0X/vl53l0x.hpp"


namespace gxm{


void ZAxisServo::setSpeed(const real_t speed){
    motor_.setSpeed(speed * config_.meter_to_turns_scaler);
};

real_t ZAxisServo::getSpeed(){
    return motor_.getSpeed() / config_.meter_to_turns_scaler;
}

real_t ZAxisServo::getErr(){
    return expect_distance - getDistance();
}
bool ZAxisServo::reached(){
    return ABS(getErr()) < config_.reached_threshold; 
}


void ZAxisServo::setDistance(const real_t distance){
    expect_distance = distance;
}

real_t ZAxisServo::getDistance(){
    return sensor_.getDistance();
}

void ZAxisServo::tick(){
    sensor_.update();

    auto pos_err = getErr();
    auto spd_err = 0 - getSpeed();

    auto spd = config_.kp * SIGN_AS(sqrt(pos_err), pos_err) - config_.kd * spd_err;
    setSpeed(spd);
}

void ZAxisServo::softHome(){
    
}
}