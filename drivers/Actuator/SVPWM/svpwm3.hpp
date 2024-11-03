#pragma once

#include "svpwm.hpp"

namespace yumud::drivers{

class SVPWM3:public SVPWM{
protected:
    using Driver = Coil3Driver;

    Driver & driver_;



public:

    SVPWM3(Driver & _driver):driver_(_driver){;}

    __inline void setDuty(const real_t modu_amp, const real_t modu_rad) override{
        scexpr real_t _30_deg = real_t(TAU / 6);
        scexpr real_t inv_30deg = real_t(6 / TAU);

        int modu_sect = (int(modu_rad * inv_30deg) % 6) + 1;
        real_t sixtant_theta = fmod(modu_rad, _30_deg);

        real_t ta = std::sin(sixtant_theta) * real_t(modu_amp);
        real_t tb = std::sin(_30_deg - sixtant_theta) * real_t(modu_amp);
        
        real_t t0 = (real_t(1) - ta - tb) / 2;
        real_t t1 = (real_t(1) + ((modu_sect % 2 == 0 )? (tb - ta) : (ta - tb))) / 2;
        real_t t2 = (real_t(1) + ta + tb) / 2;

        switch (modu_sect){
            case 1:
                driver_ = {t2, t1, t0};
                break;
            case 2:
                driver_ = {t1, t2, t0};
                break;
            case 3:
                driver_ = {t0, t2, t1};
                break;
            case 4:
                driver_ = {t0, t1, t2};
                break;
            case 5:
                driver_ = {t1, t0, t2};
                break;
            case 6:
                driver_ = {t2, t0, t1};
                break;
            default:
                break;
        }
    }
    

    void init() override{
        driver_.init();
    }

    void enable(const bool en = true) override{
        driver_.enable(en);
    }
};



}