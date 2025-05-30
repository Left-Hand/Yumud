#pragma once

#include "drivers/Actuator/Bridge/CoilDriver.hpp"

#include "core/math/realmath.hpp"


namespace ymd::digipw{
class SVPWM{
protected:
    bool rsv = false;
public:
    virtual ~SVPWM(){;}

    virtual void setDuty(const real_t duty, const real_t _elecrad) final{
        real_t elecrad = rsv ? -_elecrad : _elecrad;
        auto [s, c] = sincos(elecrad);
        set_ab_duty(c * duty, s * duty);
    }

    virtual void setDqDuty(const real_t d_duty, const real_t q_duty, const real_t _elecrad) final{
        real_t elecrad = rsv ? -_elecrad : _elecrad;
        auto [s, c] = sincos(elecrad);
        set_ab_duty(d_duty * c - q_duty * s, d_duty * c + q_duty * s);
    }

    virtual void set_ab_duty(const real_t alpha_duty, const real_t beta_duty) = 0;

    void inverse(const Enable en = EN){
        rsv = en == EN;
    }

    virtual void enable(const Enable en = EN) = 0;
};

};