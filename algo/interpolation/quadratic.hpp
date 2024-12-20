#pragma once

#include "interpolation.hpp"

namespace ymd::intp{

class QuadraticInterpolation:public Interpolation{
protected:
    Vector2 _handle;
public:
    QuadraticInterpolation(const Vector2 & handle):_handle(handle){}
    real_t forward(const real_t x);
};

}
