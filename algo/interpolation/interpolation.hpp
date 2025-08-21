#pragma once

#include "core/platform.hpp"

#include "core/math/real.hpp"
#include "types/vectors/vector2.hpp"


namespace ymd::intp{
// class Mapping{
//     // virtual real_t mapping(const Vec2 & from,const Vec2 & to, const real_t x) = 0;
// };

// class Gradient:public Mapping{

// };

class Interpolation{
protected:

public:
    virtual real_t forward(const real_t x) const = 0;
    real_t operator ()(const real_t x) const{
        return forward(x);
    }
};



}
