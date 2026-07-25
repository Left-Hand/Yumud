#pragma once

#include "core/math/fixed/fixed.hpp"

namespace ymd::math{

template<typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr int32_t clamp_int(const int32_t x, const int32_t mi, const int32_t ma){
    if (__builtin_expect(x < mi, 0)) {
        return mi;
    }
    if (__builtin_expect(x > ma, 0)) {
        return ma;
    }
    return x;
} 


template<typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr int32_t clamp2_int(const int32_t x, const int32_t side){
    return clamp_int<D>(x, -side, side);
}


template<size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, int32_t> step_to(
    math::fixed<Q, D> x, 
    math::fixed<Q, D> y, 
    math::fixed<Q, D> step
){
    const auto x_bits = clamp_int<D>(y.to_bits(), x.to_bits() - step.to_bits(), x.to_bits() + step.to_bits());
    return math::fixed<Q, D>::from_bits(x_bits);
}

template<size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, D> clamp2(math::fixed<Q, D> x, D side){
    const auto y_bits = clamp2_int<D>(x.to_bits(), side << Q);
    return math::fixed<Q, D>::from_bits(y_bits);
}


template<size_t Q, typename D>
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr math::fixed<Q, D> clamp2(math::fixed<Q, D> x, math::fixed<Q, D> side){
    const auto y_bits = clamp2_int<D>(x.to_bits(), side.to_bits());
    return math::fixed<Q, D>::from_bits(y_bits);
}




//一个数值稳定(可证明)用于求解sqrt(1 - (x)^2)的函数
//令δ(x) = sqrt(1 - (x)^2) - f(x), 可满足δ(x)恒大于0，同时使得δ(x)在考虑性能的同时足够小
//可用于对二维向量需要限定模长大小(缩放到单位圆形)的场合已知一边求解另一边
__always_inline __attribute__((const, optimize( "-Ofast" )))
static constexpr uq32 sat_ucircle(const uq32 x){
    // \left(1-x^{4}\right)+\frac{1}{2}x^{8}\cdot\left(1-x^{4}\right)
    // \left(-0.125x-0.125\right)x^{4}-\frac{1}{2}x^{2}

    const uint32_t x_u32 = x.to_bits();
    const uint32_t x2_u32 = intrinsics::mul32hu(x_u32, x_u32);
    const uint32_t x4_u32 = intrinsics::mul32hu(x2_u32, x2_u32);

    uint32_t res1 = (1 << (32 - 3));

    // -0.125x-0.125
    res1 = intrinsics::mul32hu(res1, x_u32) + (1 << (32 - 3));
    // \left(-0.125x-0.125\right)x
    res1 = intrinsics::mul32hu(res1, x4_u32);

    res1 += (x2_u32 >> 1);
    res1 = ~res1;
    
    const uint32_t one_minus_x4_u32 = ~x4_u32;
    const uint32_t x8_u32 = intrinsics::mul32hu(x4_u32, x4_u32);
    uint32_t res2 = one_minus_x4_u32;
    res2 += intrinsics::mul32hu(x8_u32 >> 1, one_minus_x4_u32);

    return uq32::from_bits(std::min(res1, res2));
}

}