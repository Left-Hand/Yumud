#include "core/math/iq/iqmath.hpp"
#include "core/math/iq/fixed_t.hpp"
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



namespace exprimental{

struct alignas(16) [[nodiscard]] IqSincosIntermediate{
    using Self = IqSincosIntermediate;

    struct SinCosResult{
        ymd::math::fixed_t<31, int32_t> sin;
        ymd::math::fixed_t<31, int32_t> cos;

    };

    uint32_t uq32_x_offset;
    int32_t iq31_sin_coeff;
    int32_t iq31_cos_coeff;
    uint8_t sect_num; 

    template<typename Fn>
    __attribute__((always_inline)) constexpr 
    ymd::math::fixed_t<31, int32_t> exact_sin(Fn && taylor_law) const {
        //获取查找表的校准值

        #if 1
        switch(sect_num){
            case 0: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 1: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));
            case 2: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));
            case 3: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 4: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
            case 5: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 6: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 7: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
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
        return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, mut_iq31_sin_coeff, mut_iq31_cos_coeff));
        #endif
    }

    template<typename Fn>
    __attribute__((always_inline)) constexpr 
    ymd::math::fixed_t<31, int32_t> exact_cos(Fn && taylor_law) const {
        switch(sect_num){
            case 0: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_cos_coeff, -iq31_sin_coeff));
            case 1: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 2: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
            case 3: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 4: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_cos_coeff,  iq31_sin_coeff));
            case 5: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, -iq31_sin_coeff, -iq31_cos_coeff));
            case 6: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
                uq32_x_offset, iq31_sin_coeff,  iq31_cos_coeff));
            case 7: return ymd::math::fixed_t<31, int32_t>::from_bits(std::forward<Fn>(taylor_law)(
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
            constexpr int32_t ONE_BY_3_IQ31_BITS = ymd::math::fixed_t<31, int32_t>(1.0/3).to_bits() + 1;
            iq31Res = iq31_mpy_uq32(ONE_BY_3_IQ31_BITS, uq32_x_offset);
            iq31Res = ymd::iqmath::details::__mpyf_l(iq31_cos_coeff, iq31Res);

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


constexpr IqSincosIntermediate __IQNgetCosSinPU(const ymd::math::fixed_t<32, uint32_t> uq32_x_pu){
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

    const int32_t iq31_sin_coeff = ymd::iqmath::details::_IQ31SinLookup[lut_index];
    const int32_t iq31_cos_coeff = ymd::iqmath::details::_IQ31CosLookup[lut_index];

    return IqSincosIntermediate{
        uq32_x_offset, 
        iq31_sin_coeff,
        iq31_cos_coeff,
        sect_num
    };

}



template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<31, int32_t> sinpu(const ymd::math::fixed_t<Q, D> iq_x){
    return __IQNgetCosSinPU<Q>(iq_x.to_bits()).exact_cos(IqSincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<31, int32_t> cospu(const ymd::math::fixed_t<Q, D> x){
    return __IQNgetCosSinPU<Q>(x.to_bits()).exact_sin(IqSincosIntermediate::exact_laws::taylor_3o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<31, int32_t> sinpu_approx(const ymd::math::fixed_t<Q, D> iq_x){
    return __IQNgetCosSinPU<Q>(iq_x.to_bits()).exact_cos(IqSincosIntermediate::exact_laws::taylor_2o);
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
ymd::math::fixed_t<31, int32_t> cospu_approx(const ymd::math::fixed_t<Q, D> x){
    return __IQNgetCosSinPU<Q>(x.to_bits()).exact_sin(IqSincosIntermediate::exact_laws::taylor_2o);
}



template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<ymd::math::fixed_t<31, int32_t>, 2> sincospu(const ymd::math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(pu_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<ymd::math::fixed_t<31, int32_t>, 2> sincos(const ymd::math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(rad_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_3o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<ymd::math::fixed_t<31, int32_t>, 2> sincospu_approx(const ymd::math::fixed_t<Q, D> x){
    const auto res = __IQNgetCosSinPU(pu_to_uq32(x)).exact_sincos(IqSincosIntermediate::exact_laws::taylor_2o);
    return {res.sin, res.cos};
}

template<size_t Q, typename D>
requires (sizeof(D) == 4)
__attribute__((always_inline)) constexpr 
std::array<ymd::math::fixed_t<31, int32_t>, 2> sincos_approx(const ymd::math::fixed_t<Q, D> x){
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


    [[nodiscard]] constexpr ymd::math::fixed_t<32, uint32_t> apply_to_uq32(uint32_t uq32_result_pu) const {
        auto & self = *this;

        /* Check if we swapped the transformation. */
        if (self.swapped) {
            /* _atan(y/x) = pi/2 - uq32_result_pu */
            uq32_result_pu = (uint32_t)(0x40000000 - uq32_result_pu);
        }

        /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
        if (self.x_is_neg) {
            /* _atan(y/x) = pi - uq32_result_pu */
            uq32_result_pu = (uint32_t)(0x80000000 - uq32_result_pu);
        }


        /* Set the sign bit and result to correct quadrant. */
        if (self.y_is_neg) {
            uq32_result_pu = ~uq32_result_pu;
        }

        return ymd::math::fixed_t<32, uint32_t>::from_bits(uq32_result_pu);
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
    // *     _atan(y/x) = pi/2 - _atan(x/y)
    // */
    static constexpr uint32_t transfrom_pu_x_to_uq32_result(uint32_t uq32_input) {
        // return 0;
        const auto * piq32Coeffs = &ymd::iqmath::details::_IQ32atan_coeffs[(uq32_input >> 25) & 0x00fc];
        /*
        * Calculate _atan(x) using the following Taylor series:
        *
        *     _atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uq32_result_pu = fast_mul(uq32_input, piq32Coeffs[0]);

        /* c3*x + c2 */
        uq32_result_pu = uq32_result_pu + piq32Coeffs[1];

        /* (c3*x + c2)*x */
        uq32_result_pu = fast_mul(uq32_input, uq32_result_pu);

        /* (c3*x + c2)*x + c1 */
        uq32_result_pu = uq32_result_pu + piq32Coeffs[2];

        /* ((c3*x + c2)*x + c1)*x */
        uq32_result_pu = fast_mul(uq32_input, uq32_result_pu);

        /* ((c3*x + c2)*x + c1)*x + c0 */
        uq32_result_pu = uq32_result_pu + piq32Coeffs[3];
        return uq32_result_pu;
    }

private:
    [[nodiscard]] __attribute__((__always_inline__)) 
    static constexpr int32_t fast_mul(uint32_t arg1, int32_t arg2){
        return uint32_t((uint64_t(arg1) * uint64_t(arg2)) >> 32);
    }
};



template<size_t Q>
constexpr ymd::math::fixed_t<32, uint32_t> atan2pu_impl(
    uint32_t uqn_y,
    uint32_t uqn_x
){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uq32_input;

    // uq32_input =0;
    // return {flag, uq32_input};

    if (uqn_y & (1U << 31)) {
        flag.y_is_neg = 1;
        uqn_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uqn_y));
    }

    if (uqn_x & (1U << 31)) {
        flag.x_is_neg = 1;
        uqn_x = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uqn_x));
    }

    /*
    * Calcualte the ratio of the inputs in ymd::math::fixed_t<31, int32_t>. When using the ymd::math::fixed_t<31, int32_t> div
    * fucntions with inputs of matching type the result will be ymd::math::fixed_t<31, int32_t>:
    *
    *     ymd::math::fixed_t<31, int32_t> = _IQ31div(iqN, iqN);
    */
    if (uqn_x < uqn_y) {
        flag.swapped = 1;
        uq32_input = std::bit_cast<uint32_t>(ymd::iqmath::details::__IQNdiv_impl<31, false>(
            uqn_x, uqn_y)) << (1);
    } else if((uqn_x > uqn_y)) {
        uq32_input = std::bit_cast<uint32_t>(ymd::iqmath::details::__IQNdiv_impl<31, false>(
            uqn_y, uqn_x)) << (1);
    } else{
        // 1/8 lap
        // 1/8 * 2^32
        return flag.apply_to_uq32(((1u << (29))));
    }
    const uint32_t uq32_result_pu = exprimental::Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}

template<size_t Q>
constexpr ymd::math::fixed_t<32, uint32_t> _atanpu_impl(
    uint32_t uqn_y
){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uq32_input;

    if (uqn_y & (1U << 31)) {
        flag.y_is_neg = 1;
        uqn_y = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uqn_y));
    }

    constexpr uint32_t ONE_BITS = (1u << Q);

    if (uqn_y > ONE_BITS) {
        flag.swapped = 1;
        //TODO 替换为更轻量的求倒数算法
        uq32_input = std::bit_cast<uint32_t>(ymd::iqmath::details::__IQNdiv_impl<31, false>(
            (1u << Q), uqn_y
        )) << 1;
    } else if(uqn_y < ONE_BITS) {
        uq32_input = uqn_y << (32 - Q);
        // uq32_input = (1u << (30));
    } else{// uqn_y == ONE_BITS
        return flag.apply_to_uq32(((1u << (29))));
    }

    const uint32_t uq32_result_pu = exprimental::Atan2Intermediate::transfrom_pu_x_to_uq32_result(uq32_input);
    return flag.apply_to_uq32(uq32_result_pu);
}


template<size_t Q>
constexpr ymd::math::fixed_t<32, uint32_t> atan2pu(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y, 
    ymd::math::fixed_t<Q, int32_t> iqn_input_x)
{
    return ymd::math::fixed_t<32, uint32_t>::from_bits(
        std::bit_cast<int32_t>(atan2pu_impl<Q>(iqn_input_y.to_bits(), iqn_input_x.to_bits()).to_bits())
    );
}






template<size_t Q>
constexpr ymd::math::fixed_t<29, int32_t> atan2(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y, 
    ymd::math::fixed_t<Q, int32_t> iqn_input_x)
{
    return ymd::math::uq32_to_rad(exprimental::atan2pu<Q>(iqn_input_y, iqn_input_x));
}

template<size_t Q>
constexpr ymd::math::fixed_t<32, uint32_t> _atanpu(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y
){
    return _atanpu_impl<Q>(iqn_input_y.to_bits());
}


template<size_t Q>
constexpr ymd::math::fixed_t<29, int32_t> _atan(
    ymd::math::fixed_t<Q, int32_t> iqn_input_y
){
    return ymd::math::uq32_to_rad(exprimental::_atanpu<Q>(iqn_input_y));
}


// constexpr auto d1 = exprimental::atan2pu(iq16(1.0), iq16(1.0)).to_bits();
// constexpr auto d1 = (double)exprimental::atan2pu(iq16(1.0), iq16(1.0));
// static_assert(Atan2Flag{0, 0, 0}.apply_to_uq32(0x40000000) == 0.25_iq31);
// static_assert(Atan2Flag{0, 1, 0}.apply_to_uq32(0x40000000) == 0.25_iq31);
// static_assert(Atan2Flag{1, 0, 0}.apply_to_uq32(0x40000000) == -0.25_iq31);
// static_assert(Atan2Flag{1, 1, 0}.apply_to_uq32(0x40000000) == -0.25_iq31);
static_assert(std::abs((double)exprimental::atan2pu(ymd::literals::iq16(1.0), ymd::literals::iq16(1.0)) - 0.125) < 1E-7);
static_assert(std::abs((double)exprimental::atan2pu(-ymd::literals::iq16(1.0), -ymd::literals::iq16(1.0)) - 0.625) < 1E-7);
static_assert(std::abs((double)exprimental::atan2pu(ymd::literals::iq16(1.0), -ymd::literals::iq16(1.0)) - 0.375) < 1E-7);
static_assert(std::abs((double)exprimental::atan2pu(-ymd::literals::iq16(1.0), ymd::literals::iq16(1.0)) - 0.875) < 1E-7);

static_assert(std::abs((double)exprimental::_atanpu(ymd::literals::iq16(1.0)) - 0.125) < 1E-7);
static_assert(std::abs((double)exprimental::_atanpu(-ymd::literals::iq16(1.0)) - 0.875) < 1E-7);

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


static constexpr ymd::math::fixed_t<32, uint32_t> iq31_length_squared(const ymd::math::fixed_t<31, int32_t> x){
    const auto abs_x_bits = x.to_bits() < 0 ? static_cast<uint32_t>(-x.to_bits()) : static_cast<uint32_t>(x.to_bits());
    const auto bits = static_cast<uint64_t>(abs_x_bits) * static_cast<uint64_t>(abs_x_bits);
    return ymd::math::fixed_t<32, uint32_t>::from_bits(static_cast<uint32_t>(bits >> 30));
}
static_assert(iq31_length_squared(ymd::math::fixed_t<31, int32_t>(0.5)) == ymd::literals::iq32(0.25));

static constexpr ymd::math::fixed_t<32, uint32_t> dual_iq31_length_squared(const ymd::math::fixed_t<31, int32_t> x, const ymd::math::fixed_t<31, int32_t> y){
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
    
    return ymd::math::fixed_t<32, uint32_t>::from_bits(static_cast<uint32_t>(shifted));
}

static_assert(dual_iq31_length_squared(ymd::literals::iq31(0.5), ymd::literals::iq31(0.5)).to_bits() == (ymd::literals::uq32(0.5)).to_bits());



}


