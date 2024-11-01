#pragma once

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

class RadianServo;

namespace gxm{

class Claw{
public:
    struct Config{
        real_t press_radian;
        real_t release_radian;
    };
    
protected:
    using Servo = RadianServo;

    const Config & config_;
    Servo & servo_;
public:
    Claw(const Config & config, Servo & servo):config_(config), servo_(servo){;}

    void press();
    void release();
};

}