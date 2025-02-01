#pragma once

#include "svpwm.hpp"

namespace ymd::drivers{

class SVPWM3:public SVPWM{
protected:
    using Driver = Coil3Driver;

    Driver & driver_;
    const uint bus_volt = 12;
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

    void setVolt(const real_t volt, const real_t modu_rad){
        setDuty(volt / bus_volt, modu_rad);
    }

    void setDuty(const real_t duty, const real_t modu_rad);

    void setAbDuty(const real_t x, const real_t y);

    void init() override{
        driver_.init();
    }

    void enable(const bool en = true) override{
        driver_.enable(en);
    }
};



}