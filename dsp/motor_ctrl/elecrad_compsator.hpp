#pragma once

#include "core/math/realmath.hpp"

namespace ymd{

struct ElecradCompensator{
    q20 base;
    uint32_t pole_pairs;

    constexpr q20 operator ()(const q20 lap_position) const {
        return (frac(frac(lap_position + base) * pole_pairs) * q20(TAU));
    }
};


}