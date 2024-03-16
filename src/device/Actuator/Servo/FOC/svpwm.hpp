#ifndef __SVPWM_H__

#define __SVPWM_H__

#include "device/Actuator/Coil/coil.hpp"

class SVPWM{
public:
    virtual void setDQCurrent(const real_t & dCurrentV, const real_t & qCurrentV, const real_t & prog) = 0;

    real_t CurrentToScale(const real_t & current){
        real_t usi_c = abs(current);
        real_t usi_v = real_t(0.8125f) - (real_t(0.1056f) / (usi_c + real_t(0.13f))) + (usi_c - real_t(0.2f)) * usi_c;
        return current > real_t(0) ? usi_v : real_t(0) - usi_v;
    }
};



class SVPWM2:public SVPWM{
protected:
    CoilConcept & coilA;
    CoilConcept & coilB;

public:
    SVPWM2(CoilConcept & _coilA, CoilConcept & _coilB):coilA(_coilA), coilB(_coilB){;}
    void init(){
        coilA.init();
        coilB.init();
    }
    void setABCurrent(const real_t & aCurrentV, const real_t & bCurrentV){
        real_t aDuty = aCurrentV / real_t(3.3f);
        real_t bDuty = bCurrentV / real_t(3.3f);
        coilA.setDuty(aDuty);
        coilB.setDuty(bDuty);
    }

    void setDQCurrent(const real_t & dCurrentV, const real_t & qCurrentV, const real_t & prog) override{
        real_t dCurrent = CurrentToScale(dCurrentV);
        real_t qCurrent = CurrentToScale(qCurrentV);
        setABCurrent(
            cos(prog) * dCurrent - sin(prog) * qCurrent,
            sin(prog) * dCurrent + cos(prog) * qCurrent
        );
    }

    void setClamp(const real_t & _clamp){
        coilA.setClamp(_clamp);
        coilB.setClamp(_clamp);
    }
};

#endif