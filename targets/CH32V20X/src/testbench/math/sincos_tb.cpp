
#include "core/math/real.hpp"
#include "core/math/iq/iqmath.hpp"
#include "core/math/iq/fixed_t.hpp"
#include "core/arch/riscv/riscv_common.hpp"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include <cmath>


/*!
 * @brief Specifies inverse square root operation type.
 */
#define TYPE_ISQRT   (0)
/*!
 * @brief Specifies square root operation type.
 */
#define TYPE_SQRT    (1)
/*!
 * @brief Specifies magnitude operation type.
 */
#define TYPE_MAG     (2)
/*!
 * @brief Specifies inverse magnitude operation type.
 */
#define TYPE_IMAG    (3)


#ifndef M_PI
#define M_PI (3.1415926536)
#endif

#define TYPE_PU         (0)
#define TYPE_RAD        (1)


using namespace ymd;


namespace exprimental{

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

template<size_t Q>
constexpr math::fixed_t<Q, uint32_t> sqrt(const math::fixed_t<Q, uint64_t> x){
    return math::fixed_t<Q, uint32_t>::from_bits(
        from_single_input_64<Q, TYPE_SQRT>(x.to_bits()
        // iqmath::details::IqSqrtCoeffs::template from_single_u32<Q, TYPE_SQRT>(x.to_bits()
        ).template compute<Q, TYPE_SQRT>()
    );
}



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



template<size_t Q, typename D>
requires (sizeof(D) == 4)
__fast_inline constexpr 
math::fixed_t<31, int32_t> sinpu(const math::fixed_t<Q, D> iq_x){
    return __IQNgetCosSinPU<Q>(iq_x.to_bits()).exact_cos(IqSincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> cospu(const math::fixed_t<Q, D> x){
    return __IQNgetCosSinPU<Q>(x.to_bits()).exact_sin(IqSincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__fast_inline constexpr 
math::fixed_t<31, int32_t> sinpu_approx(const math::fixed_t<Q, D> iq_x){
    return __IQNgetCosSinPU<Q>(iq_x.to_bits()).exact_cos(IqSincosIntermediate::exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
math::fixed_t<31, int32_t> cospu_approx(const math::fixed_t<Q, D> x){
    return __IQNgetCosSinPU<Q>(x.to_bits()).exact_sin(IqSincosIntermediate::exact_laws::taylor_2o);
}



template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincospu(const math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(pu_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincos(const math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(rad_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincospu_approx(const math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(pu_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_2o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<math::fixed_t<31, int32_t>, 2> sincos_approx(const math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(rad_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_2o);
    return {res.sin, res.cos};
}


struct [[nodiscard]] Atan2Flag{

    uint8_t y_is_neg:1;
    uint8_t x_is_neg:1;
    uint8_t swapped:1;

    static constexpr Atan2Flag zero(){
        return Atan2Flag{0, 0, 0};
    }

    static constexpr std::tuple<Atan2Flag, uint32_t> convert_to_flag(uint32_t uq31_y, uint32_t uq31_x){
        Atan2Flag flag = Atan2Flag::zero();
        uint32_t uq32_input;

        if (uq31_y & (1U << 31)) {
            flag.y_is_neg = 1;
            uq31_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uq31_y));
        }

        if (uq31_x & (1U << 31)) {
            flag.x_is_neg = 1;
            uq31_x = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uq31_x));
        }

        /*
        * Calcualte the ratio of the inputs in iq31. When using the iq31 div
        * fucntions with inputs of matching type the result will be iq31:
        *
        *     iq31 = _IQ31div(iqN, iqN);
        */
        if (uq31_x < uq31_y) {
            flag.swapped = 1;
            uq32_input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, false>(
                uq31_x, uq31_y)) << 1;
        } else if((uq31_x > uq31_y)) {
            uq32_input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, false>(
                uq31_y, uq31_x)) << 1;
        } else{
            // 1/8 lap
            // 1/8 * 2^32
            // return flag.template apply_to_uq32<Q, type>(((1u << (32 - 3))));
            uq32_input = (1u << (30));
        }
        return std::make_tuple(flag, uq32_input);
    };

    [[nodiscard]] constexpr math::fixed_t<31, int32_t> apply_to_uq32(uint32_t uq32_result_pu) const {
        auto & self = *this;
        int32_t iq31_result;

        /* Check if we swapped the transformation. */
        if (self.swapped) {
            /* atan(y/x) = pi/2 - uq32_result_pu */
            uq32_result_pu = (uint32_t)(0x40000000 - uq32_result_pu);
        }

        /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
        if (self.x_is_neg) {
            /* atan(y/x) = pi - uq32_result_pu */
            uq32_result_pu = (uint32_t)(0x80000000 - uq32_result_pu);
        }

        iq31_result = uq32_result_pu >> 1;

        /* Set the sign bit and result to correct quadrant. */
        if (self.y_is_neg) {
            return math::fixed_t<31, int32_t>::from_bits(-iq31_result);
        } else {
            return math::fixed_t<31, int32_t>::from_bits(iq31_result);
        }
    };

    [[nodiscard]] constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(*this);
    }
};

struct [[nodiscard]] Atan2Intermediate{
    using Self = Atan2Intermediate;


    // * Calculate atan2 using a 3rd order Taylor series. The coefficients are stored
    // * in a lookup table with 17 ranges to give an accuracy of XX bits.
    // *
    // * The input to the Taylor series is the ratio of the two inputs and must be
    // * in the range of 0 <= input <= 1. If the y argument is larger than the x
    // * argument we must apply the following transformation:
    // *
    // *     atan(y/x) = pi/2 - atan(x/y)
    // */
    static constexpr uint32_t transfrom_pu_x_to_uq32_result(uint32_t uq32_input) {
        const auto * piq32Coeffs = &iqmath::details::_IQ32atan_coeffs[(uq32_input >> 25) & 0x00fc];
        /*
        * Calculate atan(x) using the following Taylor series:
        *
        *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uq32_result_pu = fast_mul(uq32_input, *piq32Coeffs++);

        /* c3*x + c2 */
        uq32_result_pu = uq32_result_pu + *piq32Coeffs++;

        /* (c3*x + c2)*x */
        uq32_result_pu = fast_mul(uq32_input, uq32_result_pu);

        /* (c3*x + c2)*x + c1 */
        uq32_result_pu = uq32_result_pu + *piq32Coeffs++;

        /* ((c3*x + c2)*x + c1)*x */
        uq32_result_pu = fast_mul(uq32_input, uq32_result_pu);

        /* ((c3*x + c2)*x + c1)*x + c0 */
        uq32_result_pu = uq32_result_pu + *piq32Coeffs++;
        return uq32_result_pu;
    }

private:
    [[nodiscard]] __attribute__((__always_inline__)) 
    static constexpr int32_t fast_mul(uint32_t arg1, int32_t arg2){
        return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 32);
    }
};

template<size_t Q>
constexpr math::fixed_t<31, int32_t> atan2pu(
    math::fixed_t<Q, int32_t> iqn_input_y, 
    math::fixed_t<Q, int32_t> iqn_input_x)
{
    const auto [flag, uq32_input] = Atan2Flag::convert_to_flag(
        math::fixed_t<31, int32_t>(iqn_input_y).to_bits(), 
        math::fixed_t<31, int32_t>(iqn_input_x).to_bits()
    );
    const uint32_t uq32_result_pu = Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}

constexpr math::fixed_t<29, int32_t> iq31_to_rad(const math::fixed_t<31, int32_t> x){
    static_assert((double)std::numeric_limits<math::fixed_t<29, int32_t>>::max() > M_PI);
    static_assert((double)std::numeric_limits<math::fixed_t<29, int32_t>>::min() < -M_PI);

    constexpr uint64_t uq30_tau_bits = static_cast<uint64_t>(static_cast<long double>(
        static_cast<uint64_t>(1u) << (30)) * static_cast<long double>(M_PI * 2));

    return math::fixed_t<29, int32_t>::from_bits(
        (static_cast<int64_t>(x.to_bits()) * uq30_tau_bits) >> 32);
}

template<size_t Q>
constexpr math::fixed_t<29, int32_t> atan2(
    math::fixed_t<Q, int32_t> iqn_input_y, 
    math::fixed_t<Q, int32_t> iqn_input_x
){
    return iq31_to_rad(atan2pu(iqn_input_y, iqn_input_x));
}
template<size_t Q>
[[nodiscard]] static constexpr int32_t IQFtoN(const float fv) {
    static_assert(sizeof(float) == 4);
    constexpr uint32_t NAN_BITS = 0x7fc00000;
    const uint32_t bits = std::bit_cast<uint32_t>(fv);

    if (bits == NAN_BITS) {
        // NaN - 返回0或根据需求处理
        return 0;
    }
    
    const bool is_negative = bool(bits >> 31);
    const int32_t exponent = static_cast<int32_t>((bits >> 23) & 0xFF) - 127;
    const uint32_t mantissa_bits = bits & 0x7FFFFF;
    
    // 处理零和非常小的数
    if (exponent == -127 && mantissa_bits == 0) {
        return 0;  // 零（正或负）
    }
    
    // 检查是否超出IQ表示范围
    if (exponent >= int32_t(Q)) {
        // 溢出 - 返回最大正值或最小负值
        return (is_negative) ? 
            std::numeric_limits<int32_t>::min() : std::numeric_limits<int32_t>::max();
    }
    
    // 构建完整的尾数（包括隐含的1）
    uint64_t mantissa = static_cast<uint64_t>(mantissa_bits) | (1ULL << 23);
    
    // 调整尾数位数，考虑小数点位置
    int32_t shift = 23 - exponent - Q;
    
    int64_t result;
    if (shift >= 0) {
        result = static_cast<int64_t>(mantissa >> shift);
    } else {
        result = static_cast<int64_t>(mantissa << (-shift));
    }
    
    // 应用符号
    if(is_negative) result = -result;
    
    // 检查溢出
    if (result > std::numeric_limits<int32_t>::max()) {
        return std::numeric_limits<int32_t>::max();
    } else if (result < std::numeric_limits<int32_t>::min()) {
        return std::numeric_limits<int32_t>::min();
    }
    
    return static_cast<int32_t>(result);
}



}


static constexpr uq32 iq31_length_squared(const iq31 x){
    const auto abs_x_bits = x.to_bits() < 0 ? static_cast<uint32_t>(-x.to_bits()) : static_cast<uint32_t>(x.to_bits());
    const auto bits = static_cast<uint64_t>(abs_x_bits) * static_cast<uint64_t>(abs_x_bits);
    return uq32::from_bits(static_cast<uint32_t>(bits >> 30));
}
static_assert(iq31_length_squared(iq31(0.5)) == 0.25_uq32);

static constexpr uq32 dual_iq31_length_squared(const iq31 x, const iq31 y){
    // IQ31 格式：1 位符号 + 31 位小数
    
    // 方法1：使用数学运算（推荐）
    int32_t x_bits = x.to_bits();
    int32_t y_bits = y.to_bits();
    
    // 转换为 int64_t 进行计算，避免溢出
    int64_t x_val = static_cast<int64_t>(x_bits);  // 实际上是 Q31
    int64_t y_val = static_cast<int64_t>(y_bits);
    
    // 计算平方：Q31 × Q31 = Q62
    int64_t x_sq = (x_val * x_val);  // Q62
    int64_t y_sq = (y_val * y_val);  // Q62
    
    // 相加：Q62 + Q62 = Q62
    int64_t sum = x_sq + y_sq;  // Q62
    
    // 转换为 UQ32：从 Q62 到 Q32，需要右移 30 位
    // 但注意：sum 可能为负数（虽然理论上不会）
    if (sum < 0) {
        // 理论上不应该发生，但安全处理
        sum = 0;
    }
    
    // 右移 30 位得到 Q32（62-30=32）
    // 同时进行饱和处理
    constexpr int64_t max_uq32 = static_cast<int64_t>(std::numeric_limits<uint32_t>::max());
    int64_t shifted = sum >> 30;
    
    if (shifted > max_uq32) {
        shifted = max_uq32;
    }
    
    return uq32::from_bits(static_cast<uint32_t>(shifted));
}

static_assert(dual_iq31_length_squared(iq31(0.5), iq31(0.5)).to_bits() == (0.5_uq32).to_bits());






template<typename Fn>
__no_inline Microseconds eval_one_func(size_t times, Fn && fn){
    const auto begin_us = clock::micros();
    
    auto y = std::forward<Fn>(fn)(0, 0);
    auto x = uq32(0);
    const auto step = uq32::from_rcp(times * 4);
    for(size_t i = 0; i < times; ++i){
        auto [s,c] = math::sincos_approx(x);
        (y) += (std::forward<Fn>(fn)(s,c));
        x+= step;
    }

    const auto end_us = clock::micros();
    const auto elapsed = end_us - begin_us;
    DEBUG_PRINTLN(times, elapsed, y);
    return (end_us - begin_us);
}

template<typename Fn1, typename Fn2>
__no_inline auto compare_func(size_t times, Fn1 && fn1, Fn2 && fn2){
    const auto elapsed1 = eval_one_func(times, std::forward<Fn1>(fn1));
    const auto elapsed2 = eval_one_func(times, std::forward<Fn2>(fn2));
    DEBUG_PRINTLN(elapsed1, elapsed2);
}


template<typename Fn>
__no_inline auto eval_func(Fn && fn){
    auto y = std::forward<Fn>(fn)(clock::time());

    static constexpr size_t times = 10000;

    const auto begin_us = clock::micros();
    const auto t = clock::time();
    for(size_t i = 0; i < times; ++i){
        // __nop;
        // (y) += (std::forward<Fn>(fn)(t));
        // __nop;
        // __nop;
    }

    const auto end_us = clock::micros();
    // DEBUG_PRINTLN(static_cast<uint32_t>((end_us - begin_us).count()) / times );
    DEBUG_PRINTLN(static_cast<uint32_t>((end_us - begin_us).count()), y);
    return (end_us - begin_us);
}



template<typename Fn>
void play_func(Fn && fn){
    while(true){
        const auto now_secs = clock::time();
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        const auto x = pu_to_uq32((now_secs * 2));
        // const auto x = 6 * frac(t * 2) - 3;
        auto y = std::forward<Fn>(fn)(x);
        DEBUG_PRINTLN_IDLE(
            // x, 
            std::get<0>(y),
            std::get<1>(y)
            // std::sin(2 * M_PI * float(iq20(x))),
            // 100000 * (float(std::get<0>(y)) - std::sin(2 * M_PI * float(iq20(x)))), 
            // std::get<0>(y).to_bits() >> 16, 
            // std::get<1>(y).to_bits() >> 16, 
            // dual_iq31_length_squared(std::get<0>(y), std::get<1>(y)).to_bits() >> 16, 

            // std::get<2>(y).to_bits() >> 16, 
            // std::get<3>(y).to_bits() >> 16, 

            // dual_iq31_length_squared(std::get<2>(y), std::get<3>(y)).to_bits(), 
            // ((std::get<0>(y).to_bits() >> 16) - (std::get<2>(y).to_bits() >> 16)),
            // ((std::get<1>(y).to_bits() >> 16) - (std::get<3>(y).to_bits() >> 16)),

            // dual_iq31_length_squared((std::get<0>(y) - std::get<2>(y)),
            // (std::get<1>(y) - std::get<3>(y)).to_bits()
        );
        clock::delay(1ms);
    }
}

void sincos_main(){
    DEBUGGER_INST.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        // .baudrate = hal::NearestFreq(576_KHz), 
        .baudrate = hal::NearestFreq(6000000), 
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    // DEBUGGER.set_eps(4);
    DEBUGGER.set_eps(6);


    clock::delay(200ms);

    // const auto dur = eval_func(func);
    // PANIC{riscv_has_native_hard_f32};
    // PANIC{has_b_clz};
    // PANIC{riscv_has_native_ctz};
    // PANIC{riscv_has_native_ctz};


    // if(false)while(true){
    while(true){
        const auto now_secs = clock::time();
        // const auto x = 2 * iq16(frac(now_secs * 2)) * iq16(2 * M_PI) -  1000 * iq16(2 * M_PI);
        // const auto x = iq16(2 * M_PI) * iq16(math::frac(now_secs * 2));
        const auto x = pu_to_uq32((now_secs * 2));
        // const auto x = 6 * frac(t * 2) - 3;
        const auto [s, c] = exprimental::sincospu(x);
        // const auto [s, c] = exprimental::sincospu_approx(x);
        DEBUG_PRINTLN_IDLE(
            // x, 
            s, c, 
            math::atan2pu(iq20(s), iq20(c)),
            (int64_t)math::pu_to_uq32(exprimental::atan2pu(s, c)).to_bits() - x.to_bits()
            // iq20(exprimental::atan2(iq20(s), iq20(c)))
        );
        // clock::delay(1ms);
    }
    if(false){
        uq32 x = 0;
        iq24 y = 0;
        constexpr uq32 step = uq32::from_rcp(32u);
        for(size_t i = 0; i < 32; ++i){
            y += iq24(std::get<0>(exprimental::sincospu(x)));
            x += step;
        }
        DEBUG_PRINTLN(y);
    }

    compare_func(
        1024,
        // 32,
        [](const iq24 s, const iq24 c) -> auto {
            // const auto [s, c] = sincospu_approx(x);
            // const auto [s, c] = exprimental::sincospu_approx(x);
            // return iq20(s) + iq20(c);
            return math::atan2pu(s,c);
            // return iq20(s);
        },
        [](const iq31 s, const iq31 c) -> auto {
            // const auto [s, c] = sincospu_approx(x);
            // return exprimental::atan2pu(s,c);
            return iq31(0);
            // const auto fx = float(x);
            // const auto s = std::sin(fx);
            // const auto c = std::cos(fx);
            // return iq20::from(s) + iq20::from(c);
            // return iq20(s);
        }
    );
    PANIC{};
}