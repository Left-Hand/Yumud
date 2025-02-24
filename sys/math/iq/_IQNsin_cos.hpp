#pragma once

#include <stdint.h>

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
 *     k = 0x3F & int(Radian*64)
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
static __inline constexpr int_fast32_t __IQNcalcSin(uint_fast32_t uiq31Input){
    uint_fast16_t index;
    int_fast32_t iq31X;
    int_fast32_t iq31Sin;
    int_fast32_t iq31Cos;
    int_fast32_t iq31Res;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint_fast16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    iq31Sin = _IQ31SinLookup[index];
    iq31Cos = _IQ31CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * iq31 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    iq31X = uiq31Input & 0x01ffffff;

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

static __inline constexpr int_fast32_t __IQNcalcCos(uint_fast32_t uiq31Input)
{
    uint_fast16_t index;
    int_fast32_t iq31X;
    int_fast32_t iq31Sin;
    int_fast32_t iq31Cos;
    int_fast32_t iq31Res;

    /* Calculate index for sin and cos lookup using bits 31:26 */
    index = (uint_fast16_t)(uiq31Input >> 25) & 0x003f;

    /* Lookup S(k) and C(k) values. */
    iq31Sin = _IQ31SinLookup[index];
    iq31Cos = _IQ31CosLookup[index];

    /*
     * Calculated x (the remainder) by subtracting the index from the unsigned
     * iq31 input. This can be accomplished by masking out the bits used for
     * the index.
     */
    iq31X = uiq31Input & 0x01ffffff;

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


/**
 * @brief Computes the sine or cosine of an IQN input.
 *
 * @param iqNInput        IQN type input.
 * @param q_value         IQ format.
 * @param type            Specifies sine or cosine operation.
 * @param format          Specifies radians or per-unit operation.
 *
 * @return                IQN type result of sin or cosine operation.
 */

template<const int8_t type, const int8_t format, const int8_t q_value>
constexpr int_fast32_t __IQNsin_cos(int_fast32_t iqNInput){
    uint8_t ui8Sign = 0;
    uint_fast16_t ui16IntState;
    uint_fast16_t ui16MPYState;
    uint_fast32_t uiq29Input;
    uint_fast32_t uiq30Input;
    uint_fast32_t uiq31Input;
    uint_fast32_t uiq32Input;
    uint_fast32_t uiq31Result = 0;

    /* Remove sign from input */
    if (iqNInput < 0) {
        iqNInput = -iqNInput;

        /* Flip sign only for sin */
        if (type == TYPE_SIN) {
            ui8Sign = 1;
        }
    }

    /*
     * Mark the start of any multiplies. This will disable interrupts and set
     * the multiplier to fractional mode. This is designed to reduce overhead
     * of constantly switching states when using repeated multiplies (MSP430
     * only).
     */
    __mpyf_start(&ui16IntState, &ui16MPYState);

    /* Per unit API */
    if (format == TYPE_PU) {
        /*
         * Scale input to unsigned iq32 to allow for maximum range. This removes
         * the integer component of the per unit input.
         */
        uiq32Input = (uint_fast32_t)iqNInput << (32 - q_value);

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
        int_fast16_t exp = 29 - q_value;

        /* Save input as unsigned iq29 format. */
        uiq29Input = (uint_fast32_t)iqNInput;

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
        if (type == TYPE_COS) {
            ui8Sign ^= 1;
        }
    }

    /* Convert the unsigned iq30 input to unsigned iq31 */
    uiq31Input = uiq30Input << 1;

    /* Only one of these cases will be compiled per function. */
    if (type == TYPE_COS) {
        /* If input is greater than pi/4 use sin for calculations */
        if (uiq31Input > _iq31_quarterPi) {
            uiq31Input = _iq31_halfPi - uiq31Input;
            uiq31Result = __IQNcalcSin(uiq31Input);
        } else {
            uiq31Result = __IQNcalcCos(uiq31Input);
        }
    } else if (type == TYPE_SIN) {
        /* If input is greater than pi/4 use cos for calculations */
        if (uiq31Input > _iq31_quarterPi) {
            uiq31Input = _iq31_halfPi - uiq31Input;
            uiq31Result = __IQNcalcCos(uiq31Input);
        } else {
            uiq31Result = __IQNcalcSin(uiq31Input);
        }
    }

    /*
     * Mark the end of all multiplies. This restores MPY and interrupt states
     * (MSP430 only).
     */
    __mpy_stop(&ui16IntState, &ui16MPYState);

    /* Shift to Q type */
    uiq31Result >>= (31 - q_value);

    /* set sign */
    if (ui8Sign) {
        uiq31Result = -uiq31Result;
    }

    return uiq31Result;
}

template<int8_t q_value>
constexpr int_fast32_t _IQNsin(int_fast32_t iqNInput){
    return __IQNsin_cos<TYPE_SIN, TYPE_RAD, q_value>(iqNInput);
}


template<int8_t q_value>
constexpr int_fast32_t _IQNcos(int_fast32_t iqNInput){
    return __IQNsin_cos<TYPE_COS, TYPE_RAD, q_value>(iqNInput);
}


template<int8_t q_value>
constexpr int_fast32_t _IQNsinPU(int_fast32_t iqNInput){
    return __IQNsin_cos<TYPE_SIN, TYPE_PU, q_value>(iqNInput);
}


template<int8_t q_value>
constexpr int_fast32_t _IQNcosPU(int_fast32_t iqNInput){
    return __IQNsin_cos<TYPE_COS, TYPE_PU, q_value>(iqNInput);
}

#undef TYPE_SIN
#undef TYPE_COS
#undef TYPE_RAD
#undef TYPE_PU