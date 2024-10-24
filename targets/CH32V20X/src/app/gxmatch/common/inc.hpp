#pragma once

#include "config.hpp"


#include "types/vector3/vector3_t.hpp"
#include "types/vector2/vector2_t.hpp"

namespace gxm{
    using Vector2 = Vector2_t<real_t>;
    using Vector2i = Vector2_t<int>;

    using Vector3 = Vector3_t<real_t>;
    using Vector3i = Vector3_t<int>;
}


#include "decl/rotorinfo.hpp"
#include "decl/rowinfo.hpp"

#include "enum/index.hpp"