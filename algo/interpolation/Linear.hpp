#pragma once

#include "Interpolation.hpp"

namespace ymd::intp{

class LinearInterpolation:public Interpolation{
public:
    real_t forward(const real_t x)const override{
        return x;
    }
};

}
