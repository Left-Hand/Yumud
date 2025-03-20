#pragma once

#include "core/math/real.hpp"
#include "types/vector2/Vector2.hpp"

namespace ymd::drivers{
    class FlowSensorIntf{
    public:
        virtual Vector2_t<real_t> getPosition() = 0;
    };
}