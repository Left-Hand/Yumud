#pragma once

#include "actuator.hpp"
#include "drivers/Actuator/servo/pwm_servo/pwm_servo.hpp"

namespace gxm{

class Claw{

public:
    void press();
    void release();
};
    
}