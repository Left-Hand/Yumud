#pragma once

#include "Interpolation.hpp"

namespace ymd{

class LinearInterpolation:public Interpolation{
public:
    real_t forward(const real_t x) override{
        return x;
    }
};

}
