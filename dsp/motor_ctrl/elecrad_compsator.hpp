#pragma once

#include "core/math/realmath.hpp"
#include "core/utils/angle.hpp"

namespace ymd{

struct ElecAngleCompensator{
    uq32 base;
    uint32_t pole_pairs;

    constexpr Angle<uq32> operator ()(const Angle<uq32> machine_lap_angle) const {
        const auto lap_turns = (machine_lap_angle.to_turns());
        return Angle<uq32>::from_turns(frac(frac(lap_turns + base) * pole_pairs));
    }
};


}