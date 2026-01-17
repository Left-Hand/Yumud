#pragma once

#include "core/math/iq/fixed_t.hpp"

namespace ymd::dsp{

//a * b + c * d
template<size_t Q1, typename D1, size_t Q2, typename D2, typename ED = tmp::extended_mul_underlying_t<D1, D2>>
static constexpr math::fixed_t<Q1, D1> dot2v2(
    const math::fixed_t<Q1, D1> & a, const math::fixed_t<Q2, D2> & b,
    const math::fixed_t<Q1, D1> & c, const math::fixed_t<Q2, D2> & d
){
    ED bits = 0;
    bits += static_cast<ED>(a.to_bits()) * static_cast<ED>(b.to_bits());
    bits += static_cast<ED>(c.to_bits()) * static_cast<ED>(d.to_bits());
    return math::fixed_t<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}

//a * d - b * c
template<size_t Q1, typename D1, size_t Q2, typename D2, typename ED = tmp::extended_mul_underlying_t<D1, D2>>
static constexpr math::fixed_t<Q1, D1> cross2v2(
    const math::fixed_t<Q1, D1> & a, const math::fixed_t<Q2, D2> & b,
    const math::fixed_t<Q1, D1> & c, const math::fixed_t<Q2, D2> & d
){
    ED bits = 0;
    bits += static_cast<ED>(a.to_bits()) * static_cast<ED>(d.to_bits());
    bits -= static_cast<ED>(b.to_bits()) * static_cast<ED>(c.to_bits());
    return math::fixed_t<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}



}