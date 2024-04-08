#ifndef __SVPWM_H__

#define __SVPWM_H__

#include "device/Actuator/Coil/coil.hpp"

class SVPWM{
public:
    virtual void init() = 0;
    virtual void setDQCurrent(const real_t & dCurrentV, const real_t & qCurrentV, const real_t & prog) = 0;
    virtual void setClamp(const real_t & _clamp) = 0;
    virtual void enable(const bool & en = true) = 0;
};



class SVPWM2:public SVPWM{
protected:
    Coil2PConcept & coilA;
    Coil2PConcept & coilB;

public:
    SVPWM2(Coil2PConcept & _coilA, Coil2PConcept & _coilB):coilA(_coilA), coilB(_coilB){;}
    void init() override{
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

    void enable(const bool & en) override{
        coilA.enable(en);
        coilB.enable(en);
    }
};

// class SVPWM3:public SVPWM{
//     PwmChannel
// }

#endif