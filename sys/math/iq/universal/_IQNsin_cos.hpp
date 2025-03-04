#pragma once

#include "support.h"
#include "_IQNtables.hpp"


#ifndef PI
#define PI (3.1415926536)
#endif

/*!
 * @brief Used to specify sine operation
 */
#define TYPE_SIN     (0)
/*!
 * @brief Used to specify cosine operation
 */
#define TYPE_COS     (1)
/*!
 * @brief Used to specify result in radians
 */
#define TYPE_RAD     (0)
/*!
 * @brief Used to specify per-unit result
 */
#define TYPE_PU      (1)


namespace __iqdetails{

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

 struct __IQ32SinCos{
    int32_t uiq32Sin;
    int32_t uiq32Cos;
};

 #ifndef __IQMATH_USE_MY_IMPL

static constexpr __inline int32_t __IQ31getSin(int32_t iq31X, int32_t iq31Sin, int32_t iq31Cos){
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


static constexpr __inline int32_t __IQ31getCos(int32_t iq31X, int32_t iq31Sin, int32_t iq31Cos){
    int32_t iq31Res;

    /* 0.333*x*S(k) */
    iq31Res = __mpyf_l(0x2aaaaaab, iq31X);
    iq31Res = __mpyf_l(iq31Sin, iq31Res);

    /* -C(k) + 0.333*x*S(k) */
    iq31Res = iq31Res - iq31Cos;

    /* 0.5*x*(-C(k) + 0.333*x*S(k)) */
    iq31Res = iq31Res >> 1;
    iq31Res = __mpyf_l(iq31X, iq31Res);

    /* -S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)) */
    iq31Res = iq31Res - iq31Sin;

    /* x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k))) */
    iq31Res = __mpyf_l(iq31X, iq31Res);

    /* cos(Radian) = C(k) + x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k))) */
    iq31Res = iq31Cos + iq31Res;

    return iq31Res;
}

static __inline constexpr int32_t __IQ31calcSin(uint32_t uiq31Input){
    uint16_t index;
    int32_t iq31X;
    int32_t iq31Sin;
    int32_t iq31Cos;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    iq31Sin = _IQ31SinLookup[index];
    iq31Cos = _IQ31CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * iq31 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    iq31X = uiq31Input & 0x01ffffff;

    return __IQ31getSin(iq31X, iq31Sin, iq31Cos);
}


static __inline constexpr int32_t __IQ31calcCos(uint32_t uiq31Input)
{
    uint16_t index;
    int32_t iq31X;
    int32_t iq31Sin;
    int32_t iq31Cos;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    iq31Sin = _IQ31SinLookup[index];
    iq31Cos = _IQ31CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * iq31 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    iq31X = uiq31Input & 0x01ffffff;

    return __IQ31getCos(iq31X, iq31Sin, iq31Cos);
}

static __inline constexpr __IQ32SinCos __IQ31calcSinCos(uint32_t uiq31Input)
{
    uint16_t index;
    int32_t iq31X;
    int32_t iq31Sin;
    int32_t iq31Cos;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    iq31Sin = _IQ31SinLookup[index];
    iq31Cos = _IQ31CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * iq31 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    iq31X = uiq31Input & 0x01ffffff;

    return {__IQ31getSin(iq31X, iq31Sin, iq31Cos), __IQ31getCos(iq31X, iq31Sin, iq31Cos)};
}

#else

static __inline constexpr uint32_t __MY_GET_U32_COS(uint32_t uiq32X, uint32_t uiq32Sin, uint32_t uiq32Cos){

    /* 0.333*x*S(k) */
    uint32_t uiq32Res = __mpyf_ul32(0x2aaaaaab << 1, uiq32X);
    uiq32Res = __mpyf_ul32(uiq32Sin, uiq32Res);

    /* -S(k) - 0.333*x*S(k) */
    uiq32Res = uiq32Res - uiq32Cos;

    /* 0.5*x*(-C(k) - 0.333*x*S(k)) */
    uiq32Res = uiq32Res >> 1;
    uiq32Res = __mpyf_ul32(uiq32X, uiq32Res);

    /* -S(k) + 0.5*x*(-C(k) - 0.333*x*S(k)) */
    uiq32Res = uiq32Res - uiq32Sin;

    // x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
    uiq32Res = __mpyf_ul32(uiq32X, uiq32Res);

    // cos(Radian) = C(k) + x*(-S(k) + 0.5*x*(-C(k) + 0.333*x*S(k)))
    uiq32Res = uiq32Cos + uiq32Res;

    return uiq32Res >> 1;
}

static __inline constexpr uint32_t __MY_GET_U32_SIN(uint32_t uiq32X, uint32_t uiq32Sin, uint32_t uiq32Cos){
    /* 0.333*x*C(k) */
    uint32_t uiq32Res = __mpyf_ul32(0x2aaaaaab << 1, uiq32X);
    uiq32Res = __mpyf_ul32(uiq32Cos, uiq32Res);

    /* -S(k) - 0.333*x*C(k) */
    uiq32Res = -(uiq32Sin + uiq32Res);

    /* 0.5*x*(-S(k) - 0.333*x*C(k)) */
    uiq32Res = uiq32Res >> 1;
    uiq32Res = __mpyf_ul32(uiq32X, uiq32Res);

    /* C(k) + 0.5*x*(-S(k) - 0.333*x*C(k)) */
    uiq32Res = uiq32Cos + uiq32Res;

    /* x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
    uiq32Res = __mpyf_ul32(uiq32X, uiq32Res);

    /* sin(Radian) = S(k) + x*(C(k) + 0.5*x*(-S(k) - 0.333*x*C(k))) */
    uiq32Res = uiq32Sin + uiq32Res;

    return uiq32Res >> 1;
}

static __inline constexpr uint32_t __MY_IQNcalcSin(uint32_t uiq31Input){
    uint16_t index;
    uint32_t uiq32X;
    uint32_t uiq32Sin;
    uint32_t uiq32Cos;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    uiq32Sin = _UIQ32SinLookup[index];
    uiq32Cos = _UIQ32CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * uiq32 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    uiq32X = (uiq31Input & 0x01ffffff);
    uiq32X = uiq32X << 1;

    return __MY_GET_U32_SIN(uiq32X, uiq32Sin, uiq32Cos);
}

static __inline constexpr uint32_t __MY_IQNcalcCos(uint32_t uiq31Input){
    uint16_t index;
    uint32_t uiq32X;
    uint32_t uiq32Sin;
    uint32_t uiq32Cos;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    uiq32Sin = _UIQ32SinLookup[index];
    uiq32Cos = _UIQ32CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * uiq32 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    uiq32X = uiq31Input & 0x01ffffff;
    uiq32X = uiq32X << 1;

    return __MY_GET_U32_COS(uiq32X, uiq32Sin, uiq32Cos);
}

static __inline constexpr __UIQ32SinCos __MY_IQNcalcSinCos(uint32_t uiq31Input){
    uint16_t index;
    uint32_t uiq32X;
    uint32_t uiq32Sin;
    uint32_t uiq32Cos;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    uiq32Sin = _UIQ32SinLookup[index];
    uiq32Cos = _UIQ32CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * uiq32 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    uiq32X = uiq31Input & 0x01ffffff;
    uiq32X = uiq32X << 1;
    
    return {
        __MY_GET_U32_SIN(uiq32X, uiq32Sin, uiq32Cos),
        __MY_GET_U32_COS(uiq32X, uiq32Sin, uiq32Cos)
    };
}

#define __IQNcalcSin(x) __MY_IQNcalcSin(x)
#define __IQNcalcCos(x) __MY_IQNcalcCos(x)
#endif

/**
 * @brief Computes the sine or cosine of an IQN input.
 *
 * @param iqNInput        IQN type input.
 * @param Q         IQ format.
 * @param type            Specifies sine or cosine operation.
 * @param format          Specifies radians or per-unit operation.
 *
 * @return                IQ31 type result of sin or cosine operation.
 */

template<const int8_t Q, const int8_t type, const int8_t format>
constexpr int32_t __IQNsin_cos(int32_t iqNInput){
    uint8_t ui8Sign = 0;
    uint32_t uiq29Input;
    uint32_t uiq30Input;
    uint32_t uiq31Input;
    uint32_t uiq32Input;
    uint32_t uiq31Result = 0;

    /* Remove sign from input */
    if (iqNInput < 0) {
        iqNInput = -iqNInput;

        /* Flip sign only for sin */
        if constexpr(type == TYPE_SIN) {
            ui8Sign = 1;
        }
    }


    /* Per unit API */
    if constexpr(format == TYPE_PU) {
        /*
         * Scale input to unsigned iq32 to allow for maximum range. This removes
         * the integer component of the per unit input.
         */
        uiq32Input = (uint32_t)iqNInput << (32 - Q);

        /* Reduce the input to the first two quadrants. */
        if (uiq32Input >= 0x80000000) {
            uiq32Input -= 0x80000000;
            ui8Sign ^= 1;
        }

        /*
         * Multiply unsigned iq32 input by 2*pi and scale to unsigned iq30:
         *     iq32 * iq30 = iq30 * 2
         */
        uiq30Input = __mpyf_ul(uiq32Input, _iq30_pi);

    }
    /* Radians API */
    else {
        /* Calculate the exponent difference from input format to iq29. */
        int16_t exp = 29 - Q;

        /* Save input as unsigned iq29 format. */
        uiq29Input = (uint32_t)iqNInput;

        /* Reduce the input exponent to zero by scaling by 2*pi. */
        while (exp) {
            if (uiq29Input >= _iq29_pi) {
                uiq29Input -= _iq29_pi;
            }
            uiq29Input <<= 1;
            exp--;
        }

        /* Reduce the range to the first two quadrants. */
        if (uiq29Input >= _iq29_pi) {
            uiq29Input -= _iq29_pi;
            ui8Sign ^= 1;
        }

        /* Scale the unsigned iq29 input to unsigned iq30. */
        uiq30Input = uiq29Input << 1;
    }

    /* Reduce the iq30 input range to the first quadrant. */
    if (uiq30Input >= _iq30_halfPi) {
        uiq30Input = _iq30_pi - uiq30Input;

        /* flip sign for cos calculations */
        if constexpr(type == TYPE_COS) {
            ui8Sign ^= 1;
        }
    }

    /* Convert the unsigned iq30 input to unsigned iq31 */
    uiq31Input = uiq30Input << 1;

    /* Only one of these cases will be compiled per function. */
    if constexpr(type == TYPE_COS) {
        /* If input is greater than pi/4 use sin for calculations */
        if (uiq31Input > _iq31_quarterPi) {
            uiq31Result = __IQ31calcSin(_iq31_halfPi - uiq31Input);
        } else {
            uiq31Result = __IQ31calcCos(uiq31Input);
        }
    } else if constexpr(type == TYPE_SIN) {
        /* If input is greater than pi/4 use cos for calculations */
        if (uiq31Input > _iq31_quarterPi) {
            uiq31Result = __IQ31calcCos(_iq31_halfPi - uiq31Input);
        } else {
            uiq31Result = __IQ31calcSin(uiq31Input);
        }
    }

    /* set sign */
    if (ui8Sign) {
        return -uiq31Result;
    }else{
        return uiq31Result;
    }
}

// @param iqNInput        IQN type input.
// @return                IQ31 type result of sin or cosine operation.
template<int8_t Q>
constexpr _iq<31> _IQNsin(const _iq<Q> iqNInput){
    return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_SIN, TYPE_RAD>(iqNInput.to_i32()));
}

// @param iqNInput        IQN type input.
// @return                IQ31 type result of sin or cosine operation.
template<int8_t Q>
constexpr _iq<31> _IQNcos(const _iq<Q> iqNInput){
    return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_COS, TYPE_RAD>(iqNInput.to_i32()));
}

// @param iqNInput        IQN type input.
// @return                IQ31 type result of sin or cosine operation.
template<int8_t Q>
constexpr _iq<31> _IQNsinPU(const _iq<Q> iqNInput){
    return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_SIN, TYPE_PU>(iqNInput.to_i32()));
}


// @param iqNInput        IQN type input.
// @return                IQ31 type result of sin or cosine operation.
template<int8_t Q>
constexpr _iq<31> _IQNcosPU(const _iq<Q> iqNInput){
    return std::bit_cast<_iq<31>>(__IQNsin_cos<Q, TYPE_COS, TYPE_PU>(iqNInput.to_i32()));
}

}

#undef TYPE_SIN
#undef TYPE_COS
#undef TYPE_RAD
#undef TYPE_PU