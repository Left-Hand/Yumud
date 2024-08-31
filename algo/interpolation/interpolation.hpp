#pragma once

#include "../sys/core/system.hpp"

#include "../types/vector2/vector2_t.hpp"
#include "../types/real.hpp"

class Mapping{
    // virtual real_t mapping(const Vector2 & from,const Vector2 & to, const real_t x) = 0;
};

class Gradient:public Mapping{

};

class Interpolation:public Gradient{
public:
    template<arithmetic T, typename U>
    U liner(const T k, const U & A, const U & B){
        return k * B + (1 - k) * A;
    }

};

class LinearInterpolation:public Interpolation{
public:

};

