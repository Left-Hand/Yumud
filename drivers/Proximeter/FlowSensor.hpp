#pragma once

#include "core/math/real.hpp"
#include "types/vectors/vector2/Vector2.hpp"

namespace ymd::drivers{
    class FlowSensorIntf{
    public:
        virtual Vector2<real_t> get_position() = 0;
    };
}