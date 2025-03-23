#pragma once

#include "../Actuator.hpp"

#include "concept/pwm_channel.hpp"
#include "hal/timer/instance/timer_hw.hpp"



namespace ymd::drivers{

class Coil2DriverIntf: public ActuatorIntf{
public:
    virtual Coil2DriverIntf& operator= (const real_t duty) = 0;
};

class Coil3DriverIntf: public ActuatorIntf{

public:
    using UVW_Duty = std::tuple<real_t, real_t, real_t>;
    using AlphaBeta_Duty = std::tuple<real_t, real_t>;

    virtual Coil3DriverIntf& operator= (const UVW_Duty & duty) = 0;
};


}