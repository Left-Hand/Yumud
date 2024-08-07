#ifndef __COIL_HPP__
#define __COIL_HPP__

#include "../drivers/Actuator/Actuator.hpp"
#include "../hal/gpio/gpio.hpp"

class CoilConcept: public Actuator{
public:
    // virtual void setClamp(const real_t abs_max_value){}
    // virtual void setDuty(const real_t duty){}
};

#endif