#pragma once

#include "types/vector3/Vector3_t.hpp"
#include "types/vector2/Vector2_t.hpp"
#include "types/ray2d/Ray2D_t.hpp"

namespace gxm{

using Vector2 = ymd::Vector2_t<real_t>;
using Vector3 = ymd::Vector3_t<real_t>;

//用于描述姿态
using Gesture2D = ymd::Ray2D_t<real_t>;


}