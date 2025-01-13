#pragma once

#include "sys/core/platform.h"

namespace ymd::foc{

scexpr uint chopper_freq = 32768;
// scexpr uint chopper_freq = 65536;

using Current = real_t;

struct UvwCurrent{
    Current u, v, w;

    Current operator [](const size_t idx) const {
        return *(&u + idx);
    }

    Current & operator [](const size_t idx){
        return *(&u + idx);
    }
};

struct DqCurrent{
    Current d, q;

    Current operator [](const size_t idx) const {
        return *(&d + idx);
    }

    Current & operator [](const size_t idx){
        return *(&d + idx);
    }
};

struct AbCurrent{
    Current a, b;

    Current operator [](const size_t idx) const {
        return *(&a + idx);
    }

    Current & operator [](const size_t idx){
        return *(&b + idx);
    }
};

// using Current2 = std::array<Current, 2>;
using Voltage = real_t;


__inline auto data_to_curr(const real_t data) -> real_t{
    scexpr int res = 1000;
    scexpr real_t mul = real_t((3.3 * 9800 / res));
    return ((real_t(data) >> 4) * mul) >> 8;
};

__inline auto uvw_to_ab(const UvwCurrent & uvw) -> AbCurrent{
    scexpr real_t scale = real_t(1.731 / 2);
    return {uvw.u - ((uvw.v + uvw.w) >> 1), (uvw.w - uvw.v) * scale};
};

__inline auto ab_to_dq(const AbCurrent & ab, const real_t rad) -> DqCurrent{
    const auto s = sin(rad);
    const auto c = cos(rad);
    return {c * ab[1] - s * ab[0], s * ab[1] + c * ab[0]};
};

static __inline real_t sign_sqrt(const real_t x){
    return x < 0 ? -sqrt(-x) : sqrt(x);
};

static __inline real_t smooth(const real_t x){
    return x - sin(x);
}


}