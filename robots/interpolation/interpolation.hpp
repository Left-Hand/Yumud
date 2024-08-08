#pragma once

#include "../sys/core/system.hpp"
#include "../sys/kernel/clock.h"

#include "../types/vector2/vector2_t.hpp"

namespace Interpolation{
    struct TurnSolver{
        uint16_t ta;
        uint16_t tb;
        real_t pa;
        real_t pb;
        real_t va;
        real_t vb;
    };
    __inline real_t slopeFromT (real_t t, real_t A, real_t B, real_t C){
        real_t dtdx = real_t(1)/(3*A*t*t + 2*B*t + C); 
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
        return t * B + (1 - t) * A;
    }

    real_t cubicBezier(const real_t x, const Vector2 & a,const Vector2 & b);
    real_t ss();
}