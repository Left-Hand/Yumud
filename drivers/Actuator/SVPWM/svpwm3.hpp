#pragma once

#include "svpwm.hpp"

namespace ymd::drivers{

class SVPWM3:public SVPWM{
protected:
    using Driver = Coil3Driver;

    Driver & driver_;



public:

    SVPWM3(Driver & _driver):driver_(_driver){;}

    __inline void setDuty(const real_t modu_amp, const real_t modu_rad) override{
        scexpr real_t _30_deg = real_t(TAU / 6);
        scexpr real_t inv_30_deg = real_t(6 / TAU);

        int modu_sect = (int(modu_rad * inv_30_deg) % 6);
        real_t sixtant_theta = modu_rad - _30_deg * modu_sect;

        real_t ta = sin(sixtant_theta) * modu_amp;
        real_t tb = sin(_30_deg - sixtant_theta) * modu_amp;
        
        real_t t0 = (real_t(1) - ta - tb) >> 1;
        real_t t1 = (real_t(1) + ((modu_sect % 2) ? (tb - ta) : (ta - tb))) >> 1;
        real_t t2 = (real_t(1) + ta + tb) >> 1;

        switch (modu_sect){
            case 0:
                driver_ = {t2, t1, t0};
                break;
            case 1:
                driver_ = {t1, t2, t0};
                break;
            case 2:
                driver_ = {t0, t2, t1};
                break;
            case 3:
                driver_ = {t0, t1, t2};
                break;
            case 4:
                driver_ = {t1, t0, t2};
                break;
            case 5:
                driver_ = {t2, t0, t1};
                break;
            default:
                break;
        }
    }

    __inline void setDutyPU(const real_t modu_amp, real_t modu_rad_pu){
        scexpr real_t _30_deg = real_t(TAU / 6);
        scexpr real_t _30_deg_pu = real_t(1 / 6);

        modu_rad_pu = frac(modu_rad_pu);
        int modu_sect = (int(modu_rad_pu * 6) % 6);
        real_t sixtant_theta = (modu_rad_pu - _30_deg_pu * modu_sect) * real_t(TAU);

        real_t ta = sin(sixtant_theta) * modu_amp;
        real_t tb = sin(_30_deg - sixtant_theta) * modu_amp;
        
        real_t t0 = (real_t(1) - ta - tb) >> 1;
        real_t t1 = (real_t(1) + ((modu_sect % 2) ? (tb - ta) : (ta - tb))) >> 1;
        real_t t2 = (real_t(1) + ta + tb) >> 1;

        switch (modu_sect){
            case 0:
                driver_ = {t2, t1, t0};
                break;
            case 1:
                driver_ = {t1, t2, t0};
                break;
            case 2:
                driver_ = {t0, t2, t1};
                break;
            case 3:
                driver_ = {t0, t1, t2};
                break;
            case 4:
                driver_ = {t1, t0, t2};
                break;
            case 5:
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