#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


#ifndef PI
#define PI (3.1415926536)
#endif

#ifndef TAU
#define TAU (2 * 3.1415926536)
#endif

namespace ymd::iqmath::details{

/**
 * @brief Computes the sine of an UIQ31 input.
 *
 * @param uiq31Input      UIQ31 type input.
 *
 * @return                UIQ31 type result of sine.
 */
/*
 * Perform the calculation where the input is only in the first quadrant
 * using one of the following two functions.
 *
 * This algorithm is derived from the following trig identities:
 *     sin(k + x) = sin(k)*cos(x) + cos(k)*sin(x)
 *     cos(k + x) = cos(k)*cos(x) - sin(k)*sin(x)
 *
 * First we calculate an index k and the remainder x according to the following
 * formulas:
 *
 *     k = 0x3F & int32_t(Radian*64)
 *     x = fract(Radian*64)/64
 *
 * Two lookup tables store the values of sin(k) and cos(k) for all possible
 * indexes. The remainder, x, is calculated using second order Taylor series.
 *
 *     sin(x) = x - (x^3)/6     (~36.9 bits of accuracy)
 *     cos(x) = 1 - (x^2)/2     (~28.5 bits of accuracy)
 *
 * Combining the trig identities with the Taylor series approximiations gives
 * the following two functions:
 *
 *     cos(Radian) = C(k) + x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
 *     sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)))
 *
 *     where  S(k) = Sin table value at offset "k"
 *            C(k) = Cos table value at offset "k"
 *
 * Using a lookup table with a 64 bit index (52 indexes since the input range is
 * only 0 - 0.785398) and second order Taylor series gives 28 bits of accuracy.
 */
static constexpr __fast_inline 
int32_t __IQ31getSinCosResult(int32_t iq31X, int32_t iq31Sin, int32_t iq31Cos){
    int32_t iq31Res;

    /* 0.333*x*C(k) */
    iq31Res = __mpyf_l(0x2aaaaaab, iq31X);
    iq31Res = __mpyf_l(iq31Cos, iq31Res);

    /* -S(k) - 0.333*x*C(k) */
    iq31Res = -(iq31Sin + iq31Res);

    /* 0.5*x*(-S(k) - 0.333*x*C(k)) */
    iq31Res = iq31Res >> 1;
    iq31Res = __mpyf_l(iq31X, iq31Res);

    /* C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)) */
    iq31Res = iq31Cos + iq31Res;

    /* x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
    iq31Res = __mpyf_l(iq31X, iq31Res);

    /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
    iq31Res = iq31Sin + iq31Res;

    return iq31Res;
}


template<size_t _Q, typename Fn, size_t Q = MIN(_Q, 16)>
constexpr auto __IQNgetCosSinPUTemplate(int32_t iqn_x, Fn && fn){
    constexpr int32_t iqn_tau = (1 << Q) * (TAU);

    iqn_x = iqn_x & ((1 << Q) - 1);
    //将x取余到[0, 1)之间

    constexpr uint32_t eeq_mask = ((1 << (Q-3)) - 1);
    const uint8_t sect = iqn_x >> (Q - 3);
    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    
    const uint32_t uiqn_eeq_x = (iqn_x & eeq_mask) * (iqn_tau / 8) >> (Q - 3);
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块

    const uint32_t uiq31_eeq_x = uiqn_eeq_x << (31 - Q);
    //提高x的q值到31
    
    constexpr uint32_t uiq31_quatpi = uint32_t(uint64_t(1 << 29) * (PI));

    const uint32_t uiq31_flip_x = (sect & 0b1) ? (uiq31_quatpi - uiq31_eeq_x) : uiq31_eeq_x;
    //将x由锯齿波变为三角波

    const int32_t iq31_x = uiq31_flip_x & 0x01ffffff;
    //获取每个扇区的偏移值

    const uint8_t lut_index = (uint16_t)(uiq31_flip_x >> 25) & 0x003f;
    //计算查找表索引


    return std::forward<Fn>(fn)(iq31_x, sect, lut_index);
    // return fixed_t<31, int32_t>::from_bits(iq31_sin+iq31_cos);
}



template<size_t _Q, typename Fn, size_t Q = MIN(_Q, 16)>
constexpr auto __IQNgetCosSinTemplate(int32_t iqn_x, Fn && fn){
    if constexpr (_Q > 16) iqn_x = iqn_x >> (_Q - 16);
    constexpr int32_t iqn_tau = (1 << Q) * (TAU);
    constexpr uint32_t uiqn_inv_tau = (1 << Q) / (TAU);

    return __IQNgetCosSinPUTemplate<Q>(
        (uint32_t(iqn_x) * uiqn_inv_tau) >> Q, std::forward<Fn>(fn));
    //现在直接缩到原来1/pi 调用pu版本 这样减少了一次取余(复杂度与除法相同) 性能提高20%
    //这个函数后面的不用看了

    iqn_x = iqn_x % iqn_tau;
    if(iqn_x < 0) iqn_x += iqn_tau;
    //将x取余到[0, 2 * pi)之间

    const uint32_t uiqn_norm_x = (uint32_t(iqn_x) * uiqn_inv_tau >> Q);
    //计算x / tau的值 即为[0, 2pi) 之间到[0, 1)之前的锯齿波 以方便提取区块索引

    constexpr uint32_t eeq_mask = ((1 << (Q-3)) - 1);
    const uint8_t sect = uiqn_norm_x >> (Q - 3);
    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    
    const uint32_t uiqn_eeq_x = (uiqn_norm_x & eeq_mask) * (iqn_tau / 8) >> (Q - 3);
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块

    const uint32_t uiq31_eeq_x = uiqn_eeq_x << (31 - Q);
    //提高x的q值到31
    
    constexpr uint32_t uiq31_quatpi = uint32_t(uint64_t(1 << 29) * (PI));

    const uint32_t uiq31_flip_x = (sect & 0b1) ? (uiq31_quatpi - uiq31_eeq_x) : uiq31_eeq_x;
    //将x由锯齿波变为三角波

    const int32_t iq31_x = uiq31_flip_x & 0x01ffffff;
    //获取每个扇区的偏移值

    const uint8_t lut_index = (uint16_t)(uiq31_flip_x >> 25) & 0x003f;
    //计算查找表索引

    return std::forward<Fn>(fn)(iq31_x, sect, lut_index);
}


__fast_inline constexpr 
fixed_t<31, int32_t> __IQ31getSinDispatcher(const uint32_t iq31_x, const uint8_t sect, const uint8_t lut_index){

    const int32_t iq31_sin = iqmath::details::_IQ31SinLookup[lut_index];
    const int32_t iq31_cos = iqmath::details::_IQ31CosLookup[lut_index];
    //获取查找表的校准值

    switch(sect){
        case 0: return fixed_t<31, int32_t>::from_bits(  __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
        case 1: return fixed_t<31, int32_t>::from_bits(  __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 2: return fixed_t<31, int32_t>::from_bits(  __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 3: return fixed_t<31, int32_t>::from_bits(  __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
        case 4: return fixed_t<31, int32_t>::from_bits(- __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
        case 5: return fixed_t<31, int32_t>::from_bits(- __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 6: return fixed_t<31, int32_t>::from_bits(- __IQ31getSinCosResult(iq31_x, iq31_cos, -iq31_sin));
        case 7: return fixed_t<31, int32_t>::from_bits(- __IQ31getSinCosResult(iq31_x, iq31_sin,  iq31_cos));
    }
    __builtin_unreachable();
}

__fast_inline constexpr 
fixed_t<31, int32_t> __IQ31getCosDispatcher(const uint32_t iq31_x, const uint8_t sect, const uint8_t lut_index){
    return __IQ31getSinDispatcher(iq31_x, (sect + 2) & 0b111, lut_index);
}

__fast_inline constexpr 
auto __IQ31getSinCosDispatcher(const uint32_t iq31_x, const uint8_t sect, const uint8_t lut_index){
    struct SinCosResult{
        fixed_t<31, int32_t> sin;
        fixed_t<31, int32_t> cos;
    };

    return SinCosResult{
        __IQ31getSinDispatcher(iq31_x, sect, lut_index),
        __IQ31getCosDispatcher(iq31_x, sect, lut_index)
    };
}

}