#pragma once

#include "interpolation.hpp"

namespace ymd{

class QuadraticInterpolation:public Interpolation{
public:
    static real_t forward(const Vector2 & handle, const real_t x);
};

}
