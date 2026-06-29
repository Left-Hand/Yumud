#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::dsp{

//a * b + c * d
template<size_t Q1, typename D1, size_t Q2, typename D2>
static constexpr math::fixed<Q1, D1> dot2v2(
    const math::fixed<Q1, D1> a, const math::fixed<Q2, D2> b,
    const math::fixed<Q1, D1> c, const math::fixed<Q2, D2> d
){
    using ED = tmp::extended_mul_underlying_t<D1, D2>;
    ED bits = 0;
    bits += static_cast<ED>(a.to_bits()) * static_cast<ED>(b.to_bits());
    bits += static_cast<ED>(c.to_bits()) * static_cast<ED>(d.to_bits());
    return math::fixed<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}

//a * d - b * c
template<size_t Q1, typename D1, size_t Q2, typename D2>
static constexpr math::fixed<Q1, D1> cross2v2(
    const math::fixed<Q1, D1> a, const math::fixed<Q2, D2> b,
    const math::fixed<Q1, D1> c, const math::fixed<Q2, D2> d
){
    using ED = tmp::extended_mul_underlying_t<D1, D2>;
    ED bits = 0;
    bits += static_cast<ED>(a.to_bits()) * static_cast<ED>(d.to_bits());
    bits -= static_cast<ED>(b.to_bits()) * static_cast<ED>(c.to_bits());
    return math::fixed<Q1, D1>::from_bits(static_cast<D1>(bits >> Q2));
}

template<size_t Q>
static constexpr void inplace_resat_unit_circle(
    math::fixed<Q, int32_t> & a,
    math::fixed<Q, int32_t> & b
){
    const auto inv_mag = math::inv_mag(a, b);
    a = a * inv_mag;
    b = b * inv_mag;
}

template<size_t Q>
static constexpr std::tuple<math::fixed<Q, int32_t>, math::fixed<Q, int32_t>> 
resat_unit_circle(
    math::fixed<Q, int32_t> a,
    math::fixed<Q, int32_t> b
){
    inplace_resat_unit_circle(a, b);
    return {a ,b};
}

template<size_t Q>
static constexpr math::fixed<Q, uint32_t> heightleg(
    const math::fixed<Q, int32_t> hypotenuse,
    const math::fixed<Q, int32_t> baseside
){
    uint64_t sum = 
        uint64_t(hypotenuse.to_bits()) * hypotenuse.to_bits()
        - uint64_t(baseside.to_bits()) * baseside.to_bits()
    ;

    return math::fixed<Q, uint32_t>::from_bits(
        fxmath::details::IqSqrtIntermediate::template from_sqsum<Q, fxmath::details::SqrtNormStrategy::MAG>(sum)
        .template compute<Q, fxmath::details::SqrtNormStrategy::MAG>()
    );
}


static constexpr uint32_t lerp_pu_bits(uint32_t a0, uint32_t b, uint32_t ratio){
    uint32_t a = a0;
    if(a > b){
        std::swap(a, b);
        ratio ^= 0xffffffff;
    }

    if(b - a > 0xe000'0000){
        return uint32_t(intrinsics::mul32hsu(int32_t(b) - int32_t(a), ratio) + int32_t(a));
    }else if(b - a < 0x2000'0000){
        return intrinsics::mul32hu(uint32_t(b - a), ratio) + a;
    }else{
        return a0;
    }
}

static constexpr uq32 lerp_pu(uq32 a, uq32 b, uq32 ratio){
    return uq32::from_bits(lerp_pu_bits(a.to_bits(), b.to_bits(), ratio.to_bits()));
}



static constexpr Angular<uq32> lerp_pu_angle(Angular<uq32> a, Angular<uq32> b, uq32 ratio){
    return make_angular_from_turns(lerp_pu(a.to_turns(), b.to_turns(), ratio));
}

}