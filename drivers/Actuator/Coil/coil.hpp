#pragma once

#include "drivers/Actuator/Actuator.hpp"
#include "hal/gpio/gpio.hpp"


namespace ymd::drivers{
class CoilConcept: public Actuator{
public:
    // virtual void setClamp(const real_t abs_max_value){}
    // virtual void setDuty(const real_t duty){}
};

};

