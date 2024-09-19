#pragma once

#include "svpwm.hpp"

class SVPWM2:public SVPWM{
public:
    using Driver = Coil2Driver;

    Driver & coil_a;
    Driver & coil_b;



public:
    SVPWM2(Driver & _coilA, Driver & _coilB):coil_a(_coilA), coil_b(_coilB){;}

    void init() override{
        coil_a.init();
        coil_b.init();
    }

    void setDuty(const real_t duty, const real_t _elecrad) override{ 
        real_t elecrad = rsv ? -_elecrad : _elecrad;
        real_t alpha_duty = cos(elecrad) * duty;
        real_t beta_duty = sin(elecrad) * duty;
        
        coil_a = alpha_duty;
        coil_b = beta_duty;
    }

    void enable(const bool en = true) override{
        coil_a.enable(en);
        coil_b.enable(en);
    }
};