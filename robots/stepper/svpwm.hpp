#ifndef __SVPWM_HPP__

#define __SVPWM_HPP__

#include "drivers/Actuator/Coil/coil.hpp"
#include "types/vector2/vector2_t.hpp"

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
    virtual void setClamp(const real_t & _clamp) = 0;
    virtual void enable(const bool & en = true) = 0;
};



class SVPWM2:public SVPWM{
protected:

    using AT8222 = TB67H450;

    AT8222 & coilA;
    AT8222 & coilB;

    __fast_inline void setABCurrent(const real_t aCurrent, const real_t bCurrent){
        coilA = aCurrent;
        coilB = bCurrent;
    }

public:
    SVPWM2(AT8222 & _coilA, AT8222 & _coilB):coilA(_coilA), coilB(_coilB){;}

    __fast_inline void setCurrent(const real_t _current, const real_t _elecrad) override {
        real_t current = rsv ? -_current : _current;
        real_t cA = cos(_elecrad) * current;
        real_t cB = sin(_elecrad) * current;
        setABCurrent(cA, cB);
    }

    void init() override{
        coilA.init();
        coilB.init();
    }

    void setClamp(const real_t & _clamp) override{
        // coilA.setClamp(_clamp);
        // coilB.setClamp(_clamp);
    }

    void enable(const bool & en) override{
        coilA.enable(en);
        coilB.enable(en);
    }

    void inverse(const bool & en = true){
        rsv = true;
    }
};

// class SVPWM3:public SVPWM{
//     PwmChannel
// }

#endif