#pragma once

#include "drivers/Actuator/Bridge/CoilDriver.hpp"
#include "types/vector2/vector2_t.hpp"


namespace ymd::drivers{
class SVPWM{
protected:
    bool rsv = false;
public:
    virtual void setDuty(const real_t duty, const real_t _elecrad) final{
        real_t elecrad = rsv ? -_elecrad : _elecrad;
        setAbDuty(cos(elecrad) * duty, sin(elecrad) * duty);
    }

    // void setVolt(const real_t volt, const real_t modu_rad){
    //     setDuty(volt / bus_volt, modu_rad);
    // }

    virtual void setDqDuty(const real_t d_duty, const real_t q_duty, const real_t _elecrad) final{
        real_t elecrad = rsv ? -_elecrad : _elecrad;
        const auto c = cos(elecrad);
        const auto s = sin(elecrad);
        setAbDuty(d_duty * c - q_duty * s, d_duty * c + q_duty * s);
    }

    // void setDqVolt(const real_t volt, const real_t modu_rad){
    //     setDqDuty(volt / bus_volt, modu_rad);
    // }

    virtual void setAbDuty(const real_t alpha_duty, const real_t beta_duty) = 0;

    void inverse(const bool en = true){
        rsv = en;
    }

    virtual void enable(const bool en = true) = 0;
};

};