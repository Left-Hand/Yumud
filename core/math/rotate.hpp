#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::math{


//a * b + c * d
template<size_t Q1, typename D1, size_t Q2, typename D2>
static constexpr math::fixed<Q1, D1> dot2v2(
    const math::fixed<Q1, D1> v1x, const math::fixed<Q2, D2> v2x,
    const math::fixed<Q1, D1> v1y, const math::fixed<Q2, D2> v2y
){
    using ED = tmp::extended_mul_underlying_t<D1, D2>;
    ED bits = 0;
    bits += static_cast<ED>(v1x.to_bits()) * static_cast<ED>(v2x.to_bits());
    bits += static_cast<ED>(v1y.to_bits()) * static_cast<ED>(v2y.to_bits());
    return math::fixed<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}

//a * d - b * c
template<size_t Q1, typename D1, size_t Q2, typename D2>
static constexpr math::fixed<Q1, D1> cross2v2(
    const math::fixed<Q1, D1> v1x, const math::fixed<Q2, D2> v2x,
    const math::fixed<Q1, D1> v1y, const math::fixed<Q2, D2> v2y
){
    using ED = tmp::extended_mul_underlying_t<D1, D2>;
    ED bits = 0;
    bits += static_cast<ED>(v1x.to_bits()) * static_cast<ED>(v2y.to_bits());
    bits -= static_cast<ED>(v2x.to_bits()) * static_cast<ED>(v1y.to_bits());
    return math::fixed<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}

__attribute__((optimize("Ofast"), const, always_inline))
static constexpr std::array<int32_t, 2> rotate_int_iq31sincos(
    int32_t x, int32_t y,
    math::fixed<31, int32_t> s, 
    math::fixed<31, int32_t> c
){
    int32_t half_x_bits = 0;
    half_x_bits += intrinsics::mul32hss(x, c.to_bits());
    half_x_bits += intrinsics::mul32hss(y, -s.to_bits());

    int32_t half_y_bits = 0;
    half_y_bits += intrinsics::mul32hss(x, s.to_bits());
    half_y_bits += intrinsics::mul32hss(y, c.to_bits());

    return {
        (half_x_bits << 1),
        (half_y_bits << 1)
    };
};

__attribute__((optimize("Ofast"), const, always_inline))
static constexpr std::array<int32_t, 2> inv_rotate_int_iq31sincos(
    int32_t x, int32_t y,
    math::fixed<31, int32_t> s, 
    math::fixed<31, int32_t> c
){
    return rotate_int_iq31sincos(x, y, -s, c);
};


template<size_t Q>
__attribute__((optimize("Ofast"), const, always_inline))
static constexpr std::array<math::fixed<Q, int32_t>, 2> rotate_iq31sincos(
    math::fixed<Q, int32_t> x, 
    math::fixed<Q, int32_t> y, 
    math::fixed<31, int32_t> s, 
    math::fixed<31, int32_t> c
){
    const auto i32x2 = rotate_int_iq31sincos(
        x.to_bits(), y.to_bits(),
        s, c
    );

    return {
        math::fixed<Q, int32_t>::from_bits(i32x2[0]),
        math::fixed<Q, int32_t>::from_bits(i32x2[1])
    };
}


template<size_t Q>
__attribute__((optimize("Ofast"), const, always_inline))
static constexpr std::array<math::fixed<Q, int32_t>, 2> inv_rotate_iq31sincos(
    math::fixed<Q, int32_t> x, 
    math::fixed<Q, int32_t> y, 
    math::fixed<31, int32_t> s, 
    math::fixed<31, int32_t> c
){
    return rotate_iq31sincos(x, y, -s, c);
}





}
