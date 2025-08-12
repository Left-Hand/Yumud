#pragma once

#include "interpolation.hpp"

namespace ymd::intp{

class QuadraticInterpolation:public Interpolation{
protected:
    Vec2<q16> _handle;
public:
    QuadraticInterpolation(const Vec2<q16> & handle):_handle(handle){}
    real_t forward(const real_t x);
};

}
