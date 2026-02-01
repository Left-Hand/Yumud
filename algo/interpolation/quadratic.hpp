#pragma once

#include "interpolation.hpp"

namespace ymd::intp{

struct QuadraticInterpolation final{
    math::Vec2<iq16> handle_point;
    constexpr iq16 operator()(const iq16 x){
    // adapted from BEZMATH.PS (1993)
    // by Don Lancaster, SYNERGETICS Inc. 
    // http://www.tinaja.com/text/bezmath.html
        auto [a,b] = handle_point;
        iq16 epsilon = iq16(0.00001);
        a = MAX(0, MIN(1, a)); 
        b = MAX(0, MIN(1, b)); 

        if (a == iq16(0.5)){
            a += epsilon;
        }
        // fixed_t a = 1;
        // solve t from x (an inverse operation)
        iq16 om2a = 1 - 2*a;
        iq16 k = (math::sqrt(a*a + om2a*x) - a)/om2a;
        iq16 y = (1-2*b)*(k*k) + (2*b)*k;
        return y;
    }
};

}
