#pragma once

#include "interpolation.hpp"

class QuadraticInterpolation:public Interpolation{
public:
    static real_t mapping(const Vector2 & handle, const real_t x);
};
