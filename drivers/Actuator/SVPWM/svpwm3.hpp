#pragma once

#include "svpwm.hpp"

namespace ymd::drivers{

std::tuple<real_t, real_t, real_t> SVM(const real_t alpha, const real_t beta);

class SVPWM3 final:public SVPWM{
protected:
    using Driver = Coil3DriverIntf;

    Driver & driver_;
    const int bus_volt = 12;
    // const real_t inv_scale = real_t(1.0 / 12 * 1.15);

public:

    SVPWM3(Driver & _driver):driver_(_driver){;}

    void setDqVolt(const real_t dv, const real_t qv, const real_t rad){
        const auto c = cos(rad);
        const auto s = sin(rad);
        setAbVolt(dv * c - qv * s, dv * c + qv * s);
    }

    void setAbVolt(const real_t av, const real_t bv){
        setAbDuty(av / bus_volt, bv / bus_volt);
    }

    void setAbDuty(const real_t x, const real_t y) final override;

    void enable(const bool en = true) final override{
        driver_.enable(en);
    }
};

}