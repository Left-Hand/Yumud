#ifndef __COIL_DRIVER_HPP__
#define __COIL_DRIVER_HPP__

#include "../Actuator.hpp"

#include "../../hal/gpio/port_virtual.hpp"
#include "../../hal/timer/pwm/pwm_channel.hpp"
#include "../../hal/timer/instance/timer_hw.hpp"

#include <optional>

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
    Coil3Driver& operator= (const AlphaBeta_Duty & duty){
        auto [alpha, beta] = duty;

        real_t modu_rad = std::atan2(alpha, beta);
        real_t modu_amp = std::sqrt(alpha * alpha + beta * beta);

        int modu_sect = (int(modu_rad / real_t(TAU / 6))) % 6 + (modu_rad > 0 ? 1 : 6);
        real_t sixtant_theta = std::fmod(modu_rad, real_t(TAU / 6));
        real_t ta = modu_amp * std::sin(sixtant_theta);
        real_t tb = modu_amp * std::sin(real_t(TAU / 6) - sixtant_theta);
        
        real_t t0 = (real_t(1) - ta - tb) >> 1;
        real_t t1 = (real_t(1) + ((modu_sect % 2 == 0 )? (tb - ta) : (ta - tb))) >> 1;
        real_t t2 = (real_t(1) + ta + tb) >> 1;

        switch (modu_sect){
            case 1:
                *this = UVW_Duty(t2, t1, t0);
                break;
            case 2:
                *this = UVW_Duty(t1, t2, t0);
                break;
            case 3:
                *this = UVW_Duty(t0, t2, t1);
                break;
            case 4:
                *this = UVW_Duty(t0, t1, t2);
                break;
            case 5:
                *this = UVW_Duty(t1, t0, t2);
                break;
            case 6:
                *this = UVW_Duty(t2, t0, t1);
                break;
            default:
                break;
        }

        return *this;
    }

    virtual Coil3Driver& operator= (const UVW_Duty & duty) = 0;
};


#endif