#ifndef __SVPWM_HPP__

#define __SVPWM_HPP__

#include "../drivers/Actuator/Coil/coil.hpp"
#include "../types/vector2/vector2_t.hpp"

class SVPWM{
protected:
    bool rsv = false;
public:
    virtual void init() = 0;
    void setDQCurrent(const real_t dCurrent, const real_t qCurrent, const real_t prog){
        setDQCurrent({dCurrent, qCurrent}, prog);
    }
    void setDQCurrent(const Vector2 dqCurrent, const real_t _elecrad){
        setCurrent(dqCurrent.length_squared(), dqCurrent.angle() + _elecrad);
    }

    virtual void setCurrent(const real_t _current, const real_t _elecrad) = 0;
    virtual void setClamp(const real_t _clamp) = 0;
    virtual void enable(const bool en = true) = 0;
};



class SVPWM2:public SVPWM{
public:
    Coil2PConcept & coil_a;
    Coil2PConcept & coil_b;

    void setABCurrent(const real_t aCurrent, const real_t bCurrent){
        coil_a = aCurrent;
        coil_b = bCurrent;
    }

public:
    SVPWM2(Coil2PConcept & _coilA, Coil2PConcept & _coilB):coil_a(_coilA), coil_b(_coilB){;}

    void setCurrent(const real_t current, const real_t _elecrad) override {
        real_t elecrad = rsv ? -_elecrad : _elecrad;
        real_t cA = cos(elecrad) * current;
        real_t cB = sin(elecrad) * current;
        setABCurrent(cA, cB);
    }

    void init() override{
        coil_a.init();
        coil_b.init();
    }

    void setClamp(const real_t _clamp) override{
    }

    void enable(const bool en = true) override{
        coil_a.enable(en);
        coil_b.enable(en);
    }

    void inverse(const bool en = true){
        rsv = en;
    }
};

// class SVPWM3:public SVPWM{
//     PwmChannel
// }

#endif