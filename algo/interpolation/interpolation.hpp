#pragma once

#include "core/platform.hpp"

#include "core/math/real.hpp"
#include "types/vector2/vector2.hpp"


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
