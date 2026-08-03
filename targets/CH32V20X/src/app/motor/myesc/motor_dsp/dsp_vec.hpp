#pragma once

#include "core/math/fixed/fixed.hpp"
#include "core/math/rotate.hpp"

namespace ymd::dsp{


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



// static constexpr uint32_t lerp_pu_bits(uint32_t a0, uint32_t b, uint32_t ratio){
//     uint32_t a = a0;
//     if(a > b){
//         std::swap(a, b);
//         ratio ^= 0xffffffff;
//     }

//     if(b - a > 0xe000'0000){
//         return uint32_t(intrinsics::mul32hsu(int32_t(b) - int32_t(a), ratio) + int32_t(a));
//     }else if(b - a < 0x2000'0000){
//         return intrinsics::mul32hu(uint32_t(b - a), ratio) + a;
//     }else{
//         return a0;
//     }
// }

// static constexpr uq32 lerp_pu(uq32 a, uq32 b, uq32 ratio){
//     return uq32::from_bits(lerp_pu_bits(a.to_bits(), b.to_bits(), ratio.to_bits()));
// }



// static constexpr Angular<uq32> lerp_pu_angle(Angular<uq32> a, Angular<uq32> b, uq32 ratio){
//     return make_angular_from_turns(lerp_pu(a.to_turns(), b.to_turns(), ratio));
// }




}