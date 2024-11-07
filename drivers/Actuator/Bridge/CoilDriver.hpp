#pragma once

#include "../Actuator.hpp"

#include "hal/gpio/port_virtual.hpp"
#include "hal/timer/pwm/pwm_channel.hpp"
#include "hal/timer/instance/timer_hw.hpp"

namespace yumud::drivers{

class CoilDriverConcept: public Actuator{
public:
    // virtual void setClamp(const real_t abs_max_value){}
    // virtual void setDuty(const real_t duty){}
};

class Coil2Driver: public Actuator{
public:
    virtual Coil2Driver& operator= (const real_t duty) = 0;
};

class Coil3Driver: public Actuator{

public:
    using UVW_Duty = std::tuple<real_t, real_t, real_t>;
    using AlphaBeta_Duty = std::tuple<real_t, real_t>;

    virtual Coil3Driver& operator= (const UVW_Duty & duty) = 0;
};


}