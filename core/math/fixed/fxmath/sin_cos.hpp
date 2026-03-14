#pragma once

#include "port.hpp"
#include "constants.hpp"
#include <tuple>


#ifndef M_PI
#define M_PI (3.1415926536)
#endif


namespace ymd::fxmath::details{
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
struct alignas(16) [[nodiscard]] IqSincosIntermediate{
    using Self = IqSincosIntermediate;

    struct SinCosResult{
        math::fixed<31, int32_t> sin;
        math::fixed<31, int32_t> cos;

    };

    uint32_t uq32_x_offset;
    int32_t iq31_sin_coeff;
    int32_t iq31_cos_coeff;
    uint32_t sect_num; 



    template<typename Fn>
    __attribute__((always_inline,  optimize( "-Ofast" ))) constexpr 
    math::fixed<31, int32_t> exact_sin(Fn && taylor_law) const {
        //获取查找表的校准值
        int32_t a, b;
        switch(sect_num){
            case 0:
                a = iq31_sin_coeff;
                b =  iq31_cos_coeff;
                break;
            case 1:
                a = iq31_cos_coeff;
                b = -iq31_sin_coeff;
                break;
            case 2:
                a = iq31_cos_coeff;
                b = -iq31_sin_coeff;
                break;
            case 3:
                a = iq31_sin_coeff;
                b =  iq31_cos_coeff;
                break;
            case 4:
                a = -iq31_sin_coeff;
                b = -iq31_cos_coeff;
                break;
            case 5:
                a = -iq31_cos_coeff;
                b =  iq31_sin_coeff;
                break;
            case 6:
                a = -iq31_cos_coeff;
                b =  iq31_sin_coeff;
                break;
            case 7:
                a = -iq31_sin_coeff;
                b = -iq31_cos_coeff;
                break;
            default:
                __builtin_unreachable();
        }
        return math::fixed<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(uq32_x_offset, a, b));
    }

    template<typename Fn>
    __attribute__((always_inline,  optimize( "-Ofast" ))) constexpr 
    math::fixed<31, int32_t> exact_cos(Fn && taylor_law) const {
        int32_t a, b;
        switch(sect_num){
            case 0:
                a = iq31_cos_coeff; 
                b = -iq31_sin_coeff;
                break;
            case 1:
                a = iq31_sin_coeff; 
                b =  iq31_cos_coeff;
                break;
            case 2:
                a = -iq31_sin_coeff; 
                b = -iq31_cos_coeff;
                break;
            case 3:
                a = -iq31_cos_coeff; 
                b =  iq31_sin_coeff;
                break;
            case 4:
                a = -iq31_cos_coeff; 
                b =  iq31_sin_coeff;
                break;
            case 5:
                a = -iq31_sin_coeff; 
                b = -iq31_cos_coeff;
                break;
            case 6:
                a = iq31_sin_coeff; 
                b =  iq31_cos_coeff;
                break;
            case 7:
                a = iq31_cos_coeff; 
                b = -iq31_sin_coeff;
                break;
            default:
                __builtin_unreachable();
        }

        return math::fixed<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(uq32_x_offset, a, b));
    }

    template<typename Fn>
    __attribute__((always_inline,  optimize( "-Ofast" ))) constexpr 
    auto exact_sincos(Fn && taylor_law) const {
        return SinCosResult{
            exact_sin(std::forward<Fn>(taylor_law)),
            exact_cos(std::forward<Fn>(taylor_law))
        };
    }
};


namespace sincos_exact_laws{
    //将这些latex公式复制到desmos中打开
    //拖动x0 观察有效位数

    // s\ =\sin\left(x_{0}\right)
    // c\ =\cos\left(x_{0}\right)

    // h_{1}\left(x\right)=s\ +\ x\ \cdot\left(c\ +\ 0.5x\left(-s-\frac{1}{3}cx\right)\right)
    // h_{2}\left(x\right)=s\ +\ x\ \cdot\left(c\ +\ 0.5x\left(-s\right)\right)

    // w_{1}\left(x\right)=-\frac{\ln\left(\operatorname{abs}\left(h_{1}\left(x\right)-\sin\left(x+a\right)\right)\right)}{\ln\left(2\right)}
    // w_{2}\left(x\right)=-\frac{\ln\left(\operatorname{abs}\left(h_{2}\left(x\right)-\sin\left(x+x_{0}\right)\right)\right)}{\ln\left(2\right)}

    // res1: w_{1}\left(\frac{\pi}{4}\cdot\frac{1}{52}\right)
    // res2: w_{2}\left(\frac{\pi}{4}\cdot\frac{1}{52}\right)

    //二阶泰勒公式在x0处近似为二次方程
    //三阶泰勒公式在x0处近似为三次方程

    //通过分析发现 二阶泰勒公式能提供最低约20.73位的精度
    //通过分析发现 三阶泰勒公式能提供最低约28.78位的精度(因此精度比浮点数还高)
    __attribute__((always_inline,  optimize( "-Ofast" )))
    static constexpr int32_t 
    taylor_2o(uint32_t uq32_x_offset, int32_t iq31_sin_coeff, int32_t iq31_cos_coeff){
        int32_t res_iq31_bits = 0;

        /* -S(k) */
        res_iq31_bits = -(iq31_sin_coeff + res_iq31_bits);

        /* 0.5*x*(-S(k)) */
        res_iq31_bits = res_iq31_bits >> 1;
        res_iq31_bits = intrinsics::mul32hsu(res_iq31_bits, uq32_x_offset);

        /* C(k) + 0.5*x*(-S(k)) */
        res_iq31_bits = iq31_cos_coeff + res_iq31_bits;

        /* x*(C(k) + 0.5*x*(-S(k))) */
        res_iq31_bits = intrinsics::mul32hsu(res_iq31_bits,  uq32_x_offset);

        /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k))) */
        res_iq31_bits = iq31_sin_coeff + res_iq31_bits;

        return res_iq31_bits;
    }

    static constexpr uint32_t TWO_BY_3_UQ31 = static_cast<uint32_t>((1ull << 31) * (2.0/3)) + 1;
    static constexpr uint32_t ONE_BY_3_UQ32 = static_cast<uint32_t>((1ull << 32) * (1.0/3)) + 1;
    static_assert(TWO_BY_3_UQ31 == ONE_BY_3_UQ32);

    static constexpr uint32_t ONE_BY_3_UQ31 = static_cast<uint32_t>((1ull << 31) * (1.0/3)) + 1;

    __attribute__((always_inline,  optimize( "-Ofast" )))
    static constexpr int32_t 
    taylor_3o(uint32_t uq32_x_offset, int32_t iq31_sin_coeff, int32_t iq31_cos_coeff){
        int32_t res_iq31_bits;

        res_iq31_bits = static_cast<int32_t>(intrinsics::mul32hu(TWO_BY_3_UQ31, uq32_x_offset));
        res_iq31_bits = intrinsics::mul32hss(res_iq31_bits, iq31_cos_coeff);

        /* -S(k) - 0.333*x*C(k) */
        res_iq31_bits = -(iq31_sin_coeff + res_iq31_bits);

        /* 0.5*x*(-S(k) - 0.333*x*C(k)) */
        res_iq31_bits = res_iq31_bits >> 1;
        res_iq31_bits = intrinsics::mul32hsu(res_iq31_bits, uq32_x_offset);

        /* C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)) */
        res_iq31_bits = iq31_cos_coeff + res_iq31_bits;

        /* x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
        res_iq31_bits = intrinsics::mul32hsu(res_iq31_bits, uq32_x_offset);

        /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
        res_iq31_bits = iq31_sin_coeff + res_iq31_bits;

        return res_iq31_bits;
    }

};

__attribute__((always_inline,  optimize( "-Ofast" )))
constexpr IqSincosIntermediate make_sincospu_intermdeiate(uint32_t uq32_x_pu_bits){
    constexpr uint32_t uq32_quatpi_bits = uint32_t(((uint64_t(1u) << 32) / 4) * (M_PI));

    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    const uint32_t sect_num = static_cast<uint32_t>((uq32_x_pu_bits) >> (32 - 3));
    
    //将x由锯齿波变为三角波
    #if 0
    uq32_x_pu_bits = ((sect_num & 0b1)) ? ~uq32_x_pu_bits : uq32_x_pu_bits;
    #else
    const uint32_t inverse_mask = static_cast<uint32_t>(-(int32_t(bool(sect_num & 0b1))));
    uq32_x_pu_bits = (uq32_x_pu_bits ^ inverse_mask);
    #endif

    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块
    const uint32_t uq32_eeq_x = intrinsics::mul32hu(uq32_x_pu_bits << 3, uq32_quatpi_bits);

    //获取每个扇区的偏移值
    const uint32_t uq32_x_offset = (uq32_eeq_x)& 0x03ffffff;

    const uint32_t lut_index = uint32_t(uq32_eeq_x >> 26);
    //计算查找表索引

    // __builtin_prefetch(&fxmath::details::IQ31_SINCOS_TABLE[lut_index], 0, 3);
    const auto & pair = fxmath::details::IQ31_SINCOS_TABLE[lut_index];
    const int32_t iq31_sin_coeff = pair[0];
    const int32_t iq31_cos_coeff = pair[1];

    return fxmath::details::IqSincosIntermediate{
        uq32_x_offset, 
        iq31_sin_coeff,
        iq31_cos_coeff,
        sect_num
    };
}


}

namespace ymd::math{
template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> sin(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(rad_to_uq32(x).to_bits())
        .exact_sin(fxmath::details::sincos_exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> cos(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(rad_to_uq32(x).to_bits())
        .exact_cos(fxmath::details::sincos_exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> sin_approx(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(rad_to_uq32(x).to_bits())
        .exact_sin(fxmath::details::sincos_exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> cos_approx(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(rad_to_uq32(x).to_bits())
        .exact_cos(fxmath::details::sincos_exact_laws::taylor_2o);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> sinpu(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(pu_to_uq32(x).to_bits())
        .exact_sin(fxmath::details::sincos_exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> cospu(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(pu_to_uq32(x).to_bits())
        .exact_cos(fxmath::details::sincos_exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> sinpu_approx(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(pu_to_uq32(x).to_bits())
        .exact_sin(fxmath::details::sincos_exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
math::fixed<31, int32_t> cospu_approx(const math::fixed<Q, D> x){
    return fxmath::details::make_sincospu_intermdeiate(pu_to_uq32(x).to_bits())
        .exact_cos(fxmath::details::sincos_exact_laws::taylor_2o);
}


template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
std::array<math::fixed<31, int32_t>, 2> sincos(const math::fixed<Q, D> x){
    const auto res = fxmath::details::make_sincospu_intermdeiate(rad_to_uq32(x).to_bits())
        .exact_sincos(fxmath::details::sincos_exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
std::array<math::fixed<31, int32_t>, 2> sincospu(const math::fixed<Q, D> x){
    const auto res = fxmath::details::make_sincospu_intermdeiate(pu_to_uq32(x).to_bits())
        .exact_sincos(fxmath::details::sincos_exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
std::array<math::fixed<31, int32_t>, 2> sincospu_approx(const math::fixed<Q, D> x){
    const auto res = fxmath::details::make_sincospu_intermdeiate(pu_to_uq32(x).to_bits())
        .exact_sincos(fxmath::details::sincos_exact_laws::taylor_2o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
constexpr 
std::array<math::fixed<31, int32_t>, 2> sincos_approx(const math::fixed<Q, D> x){
    const auto res = fxmath::details::make_sincospu_intermdeiate(rad_to_uq32(x).to_bits())
        .exact_sincos(fxmath::details::sincos_exact_laws::taylor_2o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
constexpr 
fixed<16, int32_t> tan(const fixed<Q, D> x) {
    const auto [s, c] = sincos(x);
    return iq16(s) / iq16(c);
}


template<size_t Q, typename D>
constexpr 
fixed<16, int32_t> tanpu(const fixed<Q, D> x) {
    const auto [s, c] = sincospu(x);
    return iq16(s) / iq16(c);
}


//为了避免计算tan的倒数时调用了两次除法 提供cot函数
template<size_t Q, typename D>
constexpr 
fixed<16, int32_t> cot(const fixed<Q, D> x) {
    const auto [s, c] = sincos(x);
    return iq16(c) / iq16(s);
}



//为了避免计算tan的倒数时调用了两次除法 提供cot函数
template<size_t Q, typename D>
constexpr 
fixed<16, int32_t> cotpu(const fixed<Q, D> x) {
    const auto [s, c] = sincospu(x);
    return iq16(c) / iq16(s);
}

}