#pragma once

#include "interpolation.hpp"

namespace ymd::intp{

class QuadraticInterpolation:public Interpolation{
protected:
    Vector2q<16> _handle;
public:
    QuadraticInterpolation(const Vector2q<16> & handle):_handle(handle){}
    real_t forward(const real_t x);
};

}
