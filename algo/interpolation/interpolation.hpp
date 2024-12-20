#pragma once

#include "sys/core/platform.h"

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"


namespace ymd::intp{
// class Mapping{
//     // virtual real_t mapping(const Vector2 & from,const Vector2 & to, const real_t x) = 0;
// };

// class Gradient:public Mapping{

// };

class Interpolation{
protected:

public:
    using Vector2 = Vector2_t<real_t>;
    virtual real_t forward(const real_t x) const = 0;
    real_t operator ()(const real_t x) const{
        return forward(x);
    }
};



}
