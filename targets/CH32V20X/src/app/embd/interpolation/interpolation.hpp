#pragma once

#include "../sys/core/system.hpp"
#include "../sys/kernel/clock.h"

#include "../types/vector2/vector2_t.hpp"

namespace Interpolation{
    struct TurnSolver{
        uint16_t ta = 0.0f;
        uint16_t tb = 0.0f;
        real_t pa = 0.0f;
        real_t pb = 0.0f;
        real_t va = 0.0f;
        real_t vb = 0.0f;
    };
    __inline real_t slopeFromT (real_t t, real_t A, real_t B, real_t C){
        real_t dtdx = 1.0/(3.0*A*t*t + 2.0*B*t + C); 
        return dtdx;
    }

    __inline real_t xFromT (real_t t, real_t A, real_t B, real_t C, real_t D){
        real_t x = A*(t*t*t) + B*(t*t) + C*t + D;
        return x;
    }

    __inline real_t yFromT (real_t t, real_t E, real_t F, real_t G, real_t H){
        real_t y = E*(t*t*t) + F*(t*t) + G*t + H;
        return y;
    }

    __inline real_t liner(real_t t, real_t A, real_t B){
        return t * B + (1.0f - t) * A;
    }

    real_t cubicBezier(const real_t x, const Vector2 & a,const Vector2 & b);
    real_t ss();
}