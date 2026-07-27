#pragma once

#include "clamp.hpp"

namespace ymd::math{

template<size_t Q1, size_t Q2, 
    typename D1, typename D2, 
    typename D = tmp::extended_mul_underlying_t<D1, D2>
    >
static constexpr fixed<Q1 + Q2, D> compmul_extended(const fixed<Q1, D1> a, const fixed<Q2, D2> b) {
    return fixed<Q1 + Q2, D>::from_bits(static_cast<D>(a.to_bits()) * static_cast<D>(b.to_bits()));
}

template<size_t Q, size_t P>
static constexpr fixed<Q, int32_t> comp_downcast(
    const math::fixed<P, int64_t> x
){
    int32_t bits = int32_t(x.to_bits() >> (P - Q - 1));
    bits = (bits + 1) >> 1;
    return fixed<Q, int32_t>::from_bits(bits);
}

template<size_t Q, size_t P>
static constexpr math::fixed<Q, int32_t> compmul(
    const math::fixed<Q, int32_t> lhs,
    const math::fixed<P, int32_t> rhs
){
    int32_t bits = int32_t((static_cast<int64_t>(lhs.to_bits()) * rhs.to_bits()) >> (P - 1));
    bits = (bits + 1) >> 1;
    return math::fixed<Q, int32_t>::from_bits(bits);
}

template<size_t Q, size_t P>
static constexpr math::fixed<Q, int32_t> compmul_clamp2(
    const math::fixed<Q, int32_t> lhs,
    const math::fixed<P, int32_t> rhs,
    const math::fixed<Q, int32_t> ma
){
    int32_t bits = int32_t((static_cast<int64_t>(lhs.to_bits()) * rhs.to_bits()) >> (P - 1));
    bits = (bits + 1) >> 1;
    bits = math::clamp2_int<int32_t>(bits, ma.to_bits());
    return math::fixed<Q, int32_t>::from_bits(bits);
}

template<size_t Q, size_t P>
static constexpr math::fixed<Q, int32_t> compmul_clamp2(
    const math::fixed<Q, int32_t> lhs,
    const math::fixed<P, int32_t> rhs,
    const int32_t ma
){
    return compmul_clamp2(lhs, rhs, math::fixed<Q, int32_t>::from_bits(ma << Q));
}

static_assert(compmul(4.0_iq20, 3.0_iq16).to_bits() == 12 << 20);
static_assert(compmul_clamp2(4.0_iq20, 3.0_iq16, 13_iq20).to_bits() == 12 << 20);
static_assert(compmul_clamp2(4.0_iq20, 3.0_iq16, 6_iq20).to_bits() == 6 << 20);


}