#pragma once

#include "drivers/Actuator/Bridge/CoilDriver.hpp"
#include "types/vector2/vector2_t.hpp"


namespace yumud::drivers{
class SVPWM{
protected:
    bool rsv = false;
public:
    virtual void init() = 0;


    virtual void setDuty(const real_t duty, const real_t _elecrad) = 0;

    // __inline void setDQDuty(const real_t d_duty, const real_t q_duty, const real_t elecrad){
    //     setDQDuty({d_duty, q_duty}, elecrad);
    // }

    // __inline void setDQDuty(const Vector2 dq, const real_t _elecrad){
    //     // const real_t elecrad = rsv ? -_elecrad : _elecrad;
    //     const Vector2 ab = dq.rotated(_elecrad);
    //     setDuty(ab.length(), ab.angle());
    // }

    void inverse(const bool en = true){
        rsv = en;
    }

    virtual void enable(const bool en = true) = 0;
};

};