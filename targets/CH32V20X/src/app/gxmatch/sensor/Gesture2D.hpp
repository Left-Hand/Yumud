#pragma once

#include "types/vector3/Vector3_t.hpp"
#include "types/vector2/Vector2_t.hpp"

namespace gxm{
    
using Vector2 = yumud::Vector2_t<real_t>;

struct Gesture2D{
    Vector2 pos;    
    real_t rot;
};


}