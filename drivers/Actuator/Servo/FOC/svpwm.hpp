#ifndef __SVPWM_H__

#define __SVPWM_H__

#include "../drivers/Actuator/Coil/coil.hpp"

class SVPWM{
protected:
    // bool rsv = false;
public:
    virtual void init() = 0;
    virtual void setDQCurrent(const real_t & dCurrent, const real_t & qCurrent, const real_t & prog) = 0;
    virtual void setDQCurrent(const Vector2 & dqCurrent, const real_t & prog) = 0;
    virtual void setClamp(const real_t & _clamp) = 0;
    virtual void enable(const bool & en = true) = 0;
};



class SVPWM2:public SVPWM{
protected:
    Coil2PConcept & coilA;
    Coil2PConcept & coilB;

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
public:
    SVPWM2(Coil2PConcept & _coilA, Coil2PConcept & _coilB):coilA(_coilA), coilB(_coilB){;}



    void setDQCurrent(const Vector2 & dqCurrent, const real_t & prog) override{

        if(dqCurrent.x){
            const real_t progc = cos(prog);
            const real_t progs = sin(prog);
            setABCurrent(
                progc * dqCurrent.x - progs * dqCurrent.y,
                progs * dqCurrent.x + progc * dqCurrent.y
            );
        }else{
            if(dqCurrent.y){
                setABCurrent(-sin(prog) * dqCurrent.y, cos(prog) * dqCurrent.y);
            }else{
                setABCurrent(real_t(0),real_t(0));
            }
        }
    }

    void init() override{
        coilA.init();
        coilB.init();
    }

    void setClamp(const real_t & _clamp) override{
        coilA.setClamp(_clamp);
        coilB.setClamp(_clamp);
    }

    void enable(const bool & en) override{
        coilA.enable(en);
        coilB.enable(en);
    }

    // void inverse(const bool & en = true){
    //     rsv = true;
    // }
};

// class SVPWM3:public SVPWM{
//     PwmChannel
// }

#endif