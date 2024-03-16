#ifndef __SVPWM_H__

#define __SVPWM_H__

#include "device/Actuator/Coil/coil.hpp"

class SVPWM{
public:
    virtual void setDQCurrent(const real_t & dCurrentV, const real_t & qCurrentV, const real_t & prog) = 0;
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
    void setABCurrent(const real_t & aCurrent, const real_t & bCurrent){
        coilA.setDuty(aCurrent);
        coilB.setDuty(bCurrent);
    }

    void setDQCurrent(const real_t & dCurrent, const real_t & qCurrent, const real_t & prog) override{
        if(dCurrent){
            setABCurrent(
                cos(prog) * dCurrent - sin(prog) * qCurrent,
                sin(prog) * dCurrent + cos(prog) * qCurrent
            );
        }else{
            setABCurrent(-sin(prog) * qCurrent,cos(prog) * qCurrent);
        }
    }

    void setClamp(const real_t & _clamp){
        coilA.setClamp(_clamp);
        coilB.setClamp(_clamp);
    }
};

#endif