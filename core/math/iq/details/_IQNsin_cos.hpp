#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


#ifndef M_PI
#define M_PI (3.1415926536)
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
struct alignas(16) [[nodiscard]] IqSincosIntermediate{
    using Self = IqSincosIntermediate;

    struct SinCosResult{
        math::fixed_t<31, int32_t> sin;
        math::fixed_t<31, int32_t> cos;

    };

    uint32_t uq32_x_offset;
    int32_t iq31_sin_coeff;
    int32_t iq31_cos_coeff;
    uint8_t sect_num; 

    template<typename Fn>
    __attribute__((always_inline)) constexpr 
    math::fixed_t<31, int32_t> exact_sin(Fn && taylor_law) const {
        //获取查找表的校准值

        #if 1
        switch(sect_num){
            case 0: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 1: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));
            case 2: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));
            case 3: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 4: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
            case 5: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 6: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 7: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
        }
        __builtin_unreachable();
        #else
        int32_t mut_iq31_sin_coeff;
        int32_t mut_iq31_cos_coeff;
        switch(sect_num){
            case 0: 
                mut_iq31_sin_coeff = iq31_sin_coeff;
                mut_iq31_cos_coeff = iq31_cos_coeff;
                break;
            case 1: 
                mut_iq31_sin_coeff = iq31_cos_coeff;
                mut_iq31_cos_coeff = -iq31_sin_coeff;
                break;
            case 2: 
                mut_iq31_sin_coeff = iq31_cos_coeff;
                mut_iq31_cos_coeff = -iq31_sin_coeff;
                break;
            case 3: 
                mut_iq31_sin_coeff = iq31_sin_coeff;
                mut_iq31_cos_coeff =  iq31_cos_coeff;
                break;
            case 4: 
                mut_iq31_sin_coeff = -iq31_sin_coeff;
                mut_iq31_cos_coeff = -iq31_cos_coeff;
                break;
            case 5: 
                mut_iq31_sin_coeff = -iq31_cos_coeff;
                mut_iq31_cos_coeff =  iq31_sin_coeff;
                break;
            case 6: 
                mut_iq31_sin_coeff = -iq31_cos_coeff;
                mut_iq31_cos_coeff =  iq31_sin_coeff;
                break;
            case 7: 
                mut_iq31_sin_coeff = -iq31_sin_coeff;
                mut_iq31_cos_coeff = -iq31_cos_coeff;
                break;
            default:
                __builtin_unreachable();
        }
        return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, mut_iq31_sin_coeff, mut_iq31_cos_coeff));
        #endif
    }

    template<typename Fn>
    __attribute__((always_inline)) constexpr 
    math::fixed_t<31, int32_t> exact_cos(Fn && taylor_law) const {
        switch(sect_num){
            case 0: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));
            case 1: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 2: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
            case 3: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 4: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 5: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
            case 6: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 7: return math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));

        }
        __builtin_unreachable();
    }

    template<typename Fn>
    __attribute__((always_inline)) constexpr 
    auto exact_sincos(Fn && taylor_law) const {

        return SinCosResult{
            exact_sin(std::forward<Fn>(taylor_law)),
            exact_cos(std::forward<Fn>(taylor_law))
        };
    }

    struct exact_laws{
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
        static constexpr int32_t 
        taylor_2o(uint32_t uq32_x_offset, int32_t iq31_sin_coeff, int32_t iq31_cos_coeff){
            int32_t iq31Res = 0;

            /* -S(k) */
            iq31Res = -(iq31_sin_coeff + iq31Res);

            /* 0.5*x*(-S(k)) */
            iq31Res = iq31Res >> 1;
            iq31Res = iq31_mpy_uq32(iq31Res,uq32_x_offset);

            /* C(k) + 0.5*x*(-S(k)) */
            iq31Res = iq31_cos_coeff + iq31Res;

            /* x*(C(k) + 0.5*x*(-S(k))) */
            iq31Res = iq31_mpy_uq32(iq31Res, uq32_x_offset);

            /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k))) */
            iq31Res = iq31_sin_coeff + iq31Res;

            return iq31Res;
        }


        static constexpr int32_t 
        taylor_3o(uint32_t uq32_x_offset, int32_t iq31_sin_coeff, int32_t iq31_cos_coeff){
            int32_t iq31Res;

            /* 0.333*x*C(k) */
            constexpr int32_t ONE_BY_3_IQ31_BITS = math::fixed_t<31, int32_t>(1.0/3).to_bits() + 1;
            iq31Res = iq31_mpy_uq32(ONE_BY_3_IQ31_BITS, uq32_x_offset);
            iq31Res = iqmath::details::__mpyf_l(iq31_cos_coeff, iq31Res);

            /* -S(k) - 0.333*x*C(k) */
            iq31Res = -(iq31_sin_coeff + iq31Res);

            /* 0.5*x*(-S(k) - 0.333*x*C(k)) */
            iq31Res = iq31Res >> 1;
            iq31Res = iq31_mpy_uq32(iq31Res,uq32_x_offset);

            /* C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)) */
            iq31Res = iq31_cos_coeff + iq31Res;

            /* x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
            iq31Res = iq31_mpy_uq32(iq31Res, uq32_x_offset);

            /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
            iq31Res = iq31_sin_coeff + iq31Res;

            return iq31Res;
        }

    private:
        static constexpr int32_t iq31_mpy_uq32(int32_t iq31_x, uint32_t uq32_y){ 
            return static_cast<int32_t>(static_cast<int64_t>(iq31_x) * static_cast<int64_t>(uq32_y) >> 32);
        }
    };
};


constexpr IqSincosIntermediate __IQNgetCosSinPU(const math::fixed_t<32, uint32_t> uq32_x_pu){
    constexpr uint32_t uq32_quatpi_bits = uint32_t(((uint64_t(1u) << 32) / 4) * (M_PI));
    constexpr uint32_t eeq_mask = ((1 << (32-3)) - 1);

    //将一个周期拆分为八个区块 每个区块长度pi/4 获取区块索引
    const uint8_t sect_num = static_cast<uint8_t>((uq32_x_pu.to_bits()) >> (32 - 3));
    
    //将x继续塌陷 从[0, 2 * pi)变为[0, pi/4) 后期通过诱导公式映射到八个区块的任一区块
    const uint32_t uq32_eeq_x = static_cast<uint32_t>(
        (static_cast<uint64_t>(uq32_x_pu.to_bits() & eeq_mask) * (uq32_quatpi_bits)) >> (32 - 3));


    //将x由锯齿波变为三角波
    const uint32_t uq32_flip_x = (sect_num & 0b1) ? (uq32_quatpi_bits - uq32_eeq_x) : uq32_eeq_x;

    //获取每个扇区的偏移值
    const uint32_t uq32_x_offset = uq32_flip_x & 0x03ffffff;

    const uint8_t lut_index = (uint16_t)(uq32_flip_x >> 26) & 0x003f;
    //计算查找表索引

    const int32_t iq31_sin_coeff = iqmath::details::_IQ31SinLookup[lut_index];
    const int32_t iq31_cos_coeff = iqmath::details::_IQ31CosLookup[lut_index];

    return IqSincosIntermediate{
        uq32_x_offset, 
        iq31_sin_coeff,
        iq31_cos_coeff,
        sect_num
    };

}


}