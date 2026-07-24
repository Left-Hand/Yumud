#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::dsp{

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
    const auto hypotenuse_sq64 = uint64_t(int64_t(hypotenuse.to_bits()) * hypotenuse.to_bits());
    const auto baseside_sq64 = uint64_t(int64_t(baseside.to_bits()) * baseside.to_bits());
    if(hypotenuse_sq64 <= baseside_sq64) return 0;

    return math::fixed<Q, uint32_t>::from_bits(
        fxmath::details::IqSqrtIntermediate::template 
            from_sqsum64u_nonzero<Q, fxmath::details::SqrtNormStrategy::MAG>(hypotenuse_sq64 - baseside_sq64)
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


static constexpr std::tuple<uq32, int32_t> iiq32_depart(const iiq32 x){
    const uq32 prev_lap = uq32::from_bits(uint32_t(x.to_bits() & UINT32_MAX));
    int32_t floor_turns = int32_t(x.to_bits() >> 32);

    return {prev_lap, floor_turns};
} 


static constexpr iiq32 uq32_wrapped_diff(const uq32 last, const uq32 now){
    const iiq32 diff = iiq32::from_bits(
        static_cast<int64_t>(now.to_bits()) - static_cast<int64_t>(last.to_bits())
    );
    if(diff > iiq32(0.5)) return diff - 1;
    if(diff < iiq32(-0.5)) return diff + 1;
    return diff;
};

static_assert(uq32_wrapped_diff(0.35_uq32, 0.6_uq32).to_bits() == iiq32(0.25).to_bits());
static_assert(uq32_wrapped_diff(0.75_uq32, 0.50_uq32).to_bits() == iiq32(-0.25).to_bits());
static_assert(uq32_wrapped_diff(0.05_uq32, 0.80_uq32).to_bits() == iiq32(-0.25).to_bits());

static constexpr iiq32 iiq32_inc_uq32_wrapped(const iiq32 state, const uq32 last, const uq32 now){
    const auto diff = uq32_wrapped_diff(last, now);
    return state + diff;
}

// __no_inline
static constexpr iiq32 uq32_wrapped_update(
    iiq32 prev_absolute_position, 
    uq32 lap_position
){
    #if 0
    auto [prev_lap, floor_turns] = iiq32_depart(prev_absolute_position);
    const uint32_t lap_u32 = lap_position.to_bits();
    const uint32_t prev_lap_u32 = prev_lap.to_bits();

    if(lap_u32 - prev_lap_u32 > uint32_t(0x8000'0000)) [[unlikely]]{ 
        floor_turns--;
    }else if(prev_lap_u32 - lap_u32 > uint32_t(0x8000'0000)) [[unlikely]]{
        floor_turns++;
    }

    return iiq32::from_bits(int64_t(floor_turns) << 32 | lap_u32);
    #else
    // iiq32 diff = iiq32::from_bits(
    //     static_cast<int64_t>(lap_position.to_bits()) - static_cast<int64_t>(prev_absolute_position.to_bits() & UINT32_MAX)
    // );
    // if(diff > iiq32(0.5)) diff -= 1;
    // if(diff < iiq32(-0.5)) return diff += 1;
    // return prev_absolute_position + diff;

    return iiq32_inc_uq32_wrapped(
        prev_absolute_position, 
        uq32::from_bits(uint32_t(prev_absolute_position.to_bits() & UINT32_MAX)),
        lap_position);
    #endif
}

}