#pragma once

#include "core/math/realmath.hpp"
#include "primitive/arithmetic/angular.hpp"

namespace ymd{

struct ElecAngleCompensator{
    uq32 base;
    uint32_t pole_pairs;

    constexpr Angular<uq32> operator ()(const Angular<uq32> machine_lap_angle) const {
        const auto lap_turns = (machine_lap_angle.to_turns());
        return Angular<uq32>::from_turns(
            math::frac(math::frac(lap_turns + base) * pole_pairs));
    }
};


}