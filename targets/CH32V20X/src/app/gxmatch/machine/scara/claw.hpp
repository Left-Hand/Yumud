#pragma once

#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace ymd::drivers{
class RadianServo;
}

namespace gxm{

class Claw{
public:
    struct Config{
        real_t press_radian;
        real_t release_radian;
    };
    
protected:
    using Servo = ymd::drivers::RadianServo;

    const Config & config_;
    Servo & servo_;
public:
    Claw(const Config & config, Servo & servo):config_(config), servo_(servo){;}

    void press();
    void release();
};

}