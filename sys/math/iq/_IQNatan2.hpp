#pragma once

#include <stdint.h>

#include "support.h"
#include "_IQNtables.hpp"

#include "_IQNdiv.hpp"

#ifndef PI
#define PI (3.1415926536)
#endif

#define TYPE_PU         (0)
#define TYPE_RAD        (1)

#if ((!defined (__IQMATH_USE_MATHACL__)) || (!defined (__MSPM0_HAS_MATHACL__)))
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief Compute the 4-quadrant arctangent of the IQN input
 *        and return the result.
 *
 * @param iqNInputY       IQN type input y.
 * @param iqNInputX       IQN type input x.
 * @param type            Specifies radians or per-unit operation.
 * @param q_value         IQ format.
 *
 * @return                IQN type result of 4-quadrant arctangent.
 */
/*
 * Calculate atan2 using a 3rd order Taylor series. The coefficients are stored
 * in a lookup table with 17 ranges to give an accuracy of XX bits.
 *
 * The input to the Taylor series is the ratio of the two inputs and must be
 * in the range of 0 <= input <= 1. If the y argument is larger than the x
 * argument we must apply the following transformation:
 *
 *     atan(y/x) = pi/2 - atan(x/y)
 */
#if defined (__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__IQNatan2)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline=forced
#endif


template<const uint8_t type, const int8_t q_value>
constexpr int_fast32_t __IQNatan2_impl(int_fast32_t iqNInputY, int_fast32_t iqNInputX)
{
    uint8_t ui8Status = 0;
    uint8_t ui8Index;
    uint_fast16_t ui16IntState;
    uint_fast16_t ui16MPYState;
    uint_fast32_t uiqNInputX;
    uint_fast32_t uiqNInputY;
    uint_fast32_t uiq32ResultPU;
    int_fast32_t iqNResult;
    int_fast32_t iq29Result;
    const int_fast32_t *piq32Coeffs;
    uint_fast32_t uiq31Input;

    /*
     * Extract the sign from the inputs and set the following status bits:
     *
     *      ui8Status = xxxxxTQS
     *      x = unused
     *      T = transform was applied
     *      Q = 2nd or 3rd quadrant (-x)
     *      S = sign bit needs to be set (-y)
     */
    if (iqNInputY < 0) {
        ui8Status |= 1;
        iqNInputY = -iqNInputY;
    }
    if (iqNInputX < 0) {
        ui8Status |= 2;
        iqNInputX = -iqNInputX;
    }

    /* Save inputs to unsigned iqN formats. */
    uiqNInputX = (uint_fast32_t)iqNInputX;
    uiqNInputY = (uint_fast32_t)iqNInputY;

    /*
     * Calcualte the ratio of the inputs in iq31. When using the iq31 div
     * fucntions with inputs of matching type the result will be iq31:
     *
     *     iq31 = _IQ31div(iqN, iqN);
     */
    if (uiqNInputX < uiqNInputY) {
        ui8Status |= 4;
        uiq31Input = _UIQdiv<31>(uiqNInputX, uiqNInputY);
    } else {
        uiq31Input = _UIQdiv<31>(uiqNInputY, uiqNInputX);
    }

    /* Calculate the index using the left 8 most bits of the input. */
    ui8Index = (uint_fast16_t)(uiq31Input >> 24);
    ui8Index = ui8Index & 0x00fc;

    /* Set the coefficient pointer. */
    piq32Coeffs = &_IQ32atan_coeffs[ui8Index];

    /*
     * Mark the start of any multiplies. This will disable interrupts and set
     * the multiplier to fractional mode. This is designed to reduce overhead
     * of constantly switching states when using repeated multiplies (MSP430
     * only).
     */
    __mpyf_start(&ui16IntState, &ui16MPYState);

    /*
     * Calculate atan(x) using the following Taylor series:
     *
     *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
     */

    /* c3*x */
    uiq32ResultPU = __mpyf_l(uiq31Input, *piq32Coeffs++);

    /* c3*x + c2 */
    uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

    /* (c3*x + c2)*x */
    uiq32ResultPU = __mpyf_l(uiq31Input, uiq32ResultPU);

    /* (c3*x + c2)*x + c1 */
    uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

    /* ((c3*x + c2)*x + c1)*x */
    uiq32ResultPU = __mpyf_l(uiq31Input, uiq32ResultPU);

    /* ((c3*x + c2)*x + c1)*x + c0 */
    uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

    /* Check if we applied the transformation. */
    if (ui8Status & 4) {
        /* atan(y/x) = pi/2 - uiq32ResultPU */
        uiq32ResultPU = (uint32_t)(0x40000000 - uiq32ResultPU);
    }

    /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
    if (ui8Status & 2) {
        /* atan(y/x) = pi - uiq32ResultPU */
        uiq32ResultPU = (uint32_t)(0x80000000 - uiq32ResultPU);
    }

    /* Round and convert result to correct format (radians/PU and iqN type). */
    if (type == TYPE_PU) {
        uiq32ResultPU += (uint_fast32_t)1 << (31 - q_value);
        iqNResult = uiq32ResultPU >> (32 - q_value);
    } else {
        /*
         * Multiply the per-unit result by 2*pi:
         *
         *     iq31mpy(iq32, iq28) = iq29
         */
        iq29Result = __mpyf_l(uiq32ResultPU, iq28_twoPi);

        /* Only round IQ formats < 29 */
        if (q_value < 29) {
            iq29Result += (uint_fast32_t)1 << (28 - q_value);
        }
        iqNResult = iq29Result >> (29 - q_value);
    }

    /*
     * Mark the end of all multiplies. This restores MPY and interrupt states
     * (MSP430 only).
     */
    __mpy_stop(&ui16IntState, &ui16MPYState);

    /* Set the sign bit and result to correct quadrant. */
    if (ui8Status & 1) {
        return -iqNResult;
    } else {
        return iqNResult;
    }
}
#endif


template<const int8_t q_value>
constexpr int_fast32_t _IQNatan2(int_fast32_t iqNInputY, int_fast32_t iqNInputX){
    return __IQNatan2_impl<TYPE_RAD, q_value>(iqNInputY, iqNInputX);
}

template<const int8_t q_value>
constexpr int_fast32_t _IQNatan2PU(int_fast32_t iqNInputY, int_fast32_t iqNInputX){
    return __IQNatan2_impl<TYPE_PU, q_value>(iqNInputY, iqNInputX);
}


#undef TYPE_PU
#undef TYPE_RAD