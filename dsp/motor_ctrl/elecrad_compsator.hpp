#pragma once

#include "core/math/realmath.hpp"
#include "core/utils/angle.hpp"

namespace ymd{

struct ElecradCompensator{
    q20 base;
    uint32_t pole_pairs;

    constexpr Angle<q20> operator ()(const Angle<q31> lap_angle) const {
        const auto lap_position = q16(lap_angle.to_turns());
        return Angle<q20>::from_turns(frac(frac(lap_position + base) * pole_pairs));
    }
};


}