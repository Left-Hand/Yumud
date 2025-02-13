#pragma once

#include "svpwm.hpp"


namespace ymd::drivers{
class SVPWM2:public SVPWM{
public:
    using Driver = Coil2DriverIntf;

    Driver & coil_a_;
    Driver & coil_b_;

public:
    SVPWM2(Driver & coil_a, Driver & coil_b):coil_a_(coil_a), coil_b_(coil_b){;}


    __fast_inline void setAbDuty(const real_t duty_a, const real_t duty_b) override{
        coil_a_ = duty_a;
        coil_b_ = duty_b;
    }

    void enable(const bool en = true) override{
        coil_a_.enable(en);
        coil_b_.enable(en);
    }
};

}