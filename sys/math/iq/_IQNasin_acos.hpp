#pragma once

#include <stdint.h>

#include "support.h"
#include "_IQNtables.hpp"

#include "_IQNsqrt.hpp"

/* Hidden Q31 sqrt function. */
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief Computes the inverse sine of the IQN input.
 *
 * @param iqNInput        IQN type input.
 * @param q_value         IQ format.
 *
 * @return                IQN type result of inverse sine.
 */
/*
 * Calculate asin using a 4th order Taylor series for inputs in the range of
 * zero to 0.5. The coefficients are stored in a lookup table with 17 ranges
 * to give an accuracy of 26 bits.
 *
 * For inputs greater than 0.5 we apply the following transformation:
 *
 *     asin(x) = PI/2 - 2*asin(sqrt((1 - x)/2))
 *
 * This transformation is derived from the following trig identities:
 *
 *     (1) asin(x) = PI/2 - acos(x)
 *     (2) sin(t/2)^2 = (1 - cos(t))/2
 *     (3) cos(t) = x
 *     (4) t = acos(x)
 *
 * Identity (2) can be simplified to give equation (5):
 *
 *     (5) t = 2*asin(sqrt((1 - cos(t))/2))
 *
 * Substituing identities (3) and (4) into equation (5) gives equation (6):
 *
 *     (6) acos(x) = 2*asin(sqrt((1 - x)/2))
 *
 * The final step is substituting equation (6) into identity (1):
 *
 *     asin(x) = PI/2 - 2*asin(sqrt((1 - x)/2))
 *
 * Acos is implemented using asin and identity (1).
 */

template<const int8_t q_value>
constexpr int_fast32_t _IQNasin(int_fast32_t iqNInput)
{
    uint8_t ui8Status = 0;
    uint_fast16_t index;
    uint_fast16_t ui16IntState;
    uint_fast16_t ui16MPYState;
    int_fast32_t iq29Result;
    const int_fast32_t *piq29Coeffs;
    uint_fast32_t uiq31Input;
    uint_fast32_t uiq31InputTemp;

    /*
     * Extract the sign from the input and set the following status bits:
     *
     *      ui8Status = xxxxxxTS
     *      x = unused
     *      T = transform was applied
     *      S = sign bit needs to be set (-y)
     */
    if (iqNInput < 0) {
        ui8Status |= 1;
        iqNInput = -iqNInput;
    }

    /*
     * Check if input is within the valid input range:
     *
     *     0 <= iqNInput <= 1
     */
    if (iqNInput > ((int_fast32_t)1 << q_value)) {
        return 0;
    }

    /* Convert input to unsigned iq31. */
    uiq31Input = (uint_fast32_t)iqNInput << (31 - q_value);

    /*
     * Apply the transformation from asin to acos if input is greater than 0.5.
     * The first step is to calculate the following:
     *
     *     (sqrt((1 - uiq31Input)/2))
     */
    uiq31InputTemp = 0x80000000 - uiq31Input;
    if (uiq31InputTemp < 0x40000000) {
        /* Halve the result. */
        uiq31Input = uiq31InputTemp >> 1;

        /* Calculate sqrt((1 - uiq31Input)/2) */
        uiq31Input = _IQNsqrt<31>(uiq31Input);

        /* Flag that the transformation was used. */
        ui8Status |= 2;
    }

    /* Calculate the index using the left 6 most bits of the input. */
    index = (int_fast16_t)(uiq31Input >> 26) & 0x003f;

    /* Set the coefficient pointer. */
    piq29Coeffs = _IQ29Asin_coeffs[index];

    /*
     * Mark the start of any multiplies. This will disable interrupts and set
     * the multiplier to fractional mode. This is designed to reduce overhead
     * of constantly switching states when using repeated multiplies (MSP430
     * only).
     */
    __mpyf_start(&ui16IntState, &ui16MPYState);

    /*
     * Calculate asin(x) using the following Taylor series:
     *
     *     asin(x) = (((c4*x + c3)*x + c2)*x + c1)*x + c0
     */

    /* c4*x */
    iq29Result = __mpyf_l(uiq31Input, *piq29Coeffs++);

    /* c4*x + c3 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* (c4*x + c3)*x */
    iq29Result = __mpyf_l(uiq31Input, iq29Result);

    /* (c4*x + c3)*x + c2 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* ((c4*x + c3)*x + c2)*x */
    iq29Result = __mpyf_l(uiq31Input, iq29Result);

    /* ((c4*x + c3)*x + c2)*x + c1 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /* (((c4*x + c3)*x + c2)*x + c1)*x */
    iq29Result = __mpyf_l(uiq31Input, iq29Result);

    /* (((c4*x + c3)*x + c2)*x + c1)*x + c0 */
    iq29Result = iq29Result + *piq29Coeffs++;

    /*
     * Mark the end of all multiplies. This restores MPY and interrupt states
     * (MSP430 only).
     */
    __mpy_stop(&ui16IntState, &ui16MPYState);

    /* check if we switched to acos */
    if (ui8Status & 2) {
        /* asin(x) = pi/2 - 2*iq29Result */
        iq29Result = iq29Result << 1;
        iq29Result -= iq29_halfPi;      // this is equivalent to the above
        iq29Result = -iq29Result;       // but avoids using temporary registers
    }

    /* Shift iq29 result to specified q_value. */
    iq29Result >>= 29 - q_value;

    /* Add sign to the result. */
    if (ui8Status & 1) {
        iq29Result = -iq29Result;
    }

    return iq29Result;
}
