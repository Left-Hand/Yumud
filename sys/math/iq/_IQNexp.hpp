#pragma once

#include <stdint.h>

#include "support.h"
#include "_IQNtables.hpp"


/**
 * @brief Computes the exponential of an IQN input.
 *
 * @param iqNInput          IQN type input.
 * @param iqNLookupTable    Integer result lookup table.
 * @param ui8IntegerOffset  Integer portion offset
 * @param iqN_MIN           Minimum parameter value.
 * @param iqN_MAX           Maximum parameter value.
 * @param q_value           IQ format.
 *
 *
 * @return                  IQN type result of exponential.
 */

template<const int8_t q_value>
constexpr int_fast32_t _IQNexp(int_fast32_t iqNInput){
    const uint_fast32_t *iqNLookupTable = _IQNexp_lookup[q_value - 1];
    uint8_t ui8IntegerOffset = _IQNexp_offset[q_value - 1];
    const int_fast32_t iqN_MIN = _IQNexp_min[q_value - 1];
    const int_fast32_t iqN_MAX = _IQNexp_max[q_value - 1];
    
    uint8_t ui8Count;
    int_fast16_t i16Integer;
    uint_fast16_t ui16IntState;
    uint_fast16_t ui16MPYState;
    int_fast32_t iq31Fractional;
    uint_fast32_t uiqNResult;
    uint_fast32_t uiqNIntegerResult;
    uint_fast32_t uiq30FractionalResult;
    uint_fast32_t uiq31FractionalResult;
    const uint_fast32_t *piq30Coeffs;

    /* Input is negative. */
    if (iqNInput < 0) {
        /* Check for the minimum value. */
        if (iqNInput < iqN_MIN) {
            return 0;
        }

        /* Extract the fractional portion in iq31 and set sign bit. */
        iq31Fractional = iqNInput;
        iq31Fractional <<= (31 - q_value);
        iq31Fractional |= 0x80000000;

        /* Extract the integer portion. */
        i16Integer = (int_fast16_t)(iqNInput >> q_value) + 1;

        /* Offset the integer portion and lookup the integer result. */
        i16Integer += ui8IntegerOffset;
        uiqNIntegerResult = iqNLookupTable[i16Integer];

        /* Reduce the fractional portion to -ln(2) < iq31Fractional < 0 */
        if (iq31Fractional <= -iq31_ln2) {
            iq31Fractional += iq31_ln2;
            uiqNIntegerResult >>= 1;
        }
    }
    /* Input is positive. */
    else {
        /* Check for the maximum value. */
        if (iqNInput > iqN_MAX) {
            return INT32_MAX;
        }

        /* Extract the fractional portion in iq31 and clear sign bit. */
        iq31Fractional = iqNInput;
        iq31Fractional <<= (31 - q_value);
        iq31Fractional &= 0x7fffffff;

        /* Extract the integer portion. */
        i16Integer = (int_fast16_t)(iqNInput >> q_value);

        /* Offset the integer portion and lookup the integer result. */
        i16Integer += ui8IntegerOffset;
        uiqNIntegerResult = iqNLookupTable[i16Integer];

        /* Reduce the fractional portion to 0 < iq31Fractional < ln(2) */
        if (iq31Fractional >= iq31_ln2) {
            iq31Fractional -= iq31_ln2;
            uiqNIntegerResult <<= 1;
        }
    }

    /*
     * Mark the start of any multiplies. This will disable interrupts and set
     * the multiplier to fractional mode. This is designed to reduce overhead
     * of constantly switching states when using repeated multiplies (MSP430
     * only).
     */
    __mpyf_start(&ui16IntState, &ui16MPYState);

    /*
     * Initialize the coefficient pointer to the Taylor Series iq30 coefficients
     * for the exponential functions. Set the iq30 result to the first
     * coefficient in the table.
     */
    piq30Coeffs = _IQ30exp_coeffs;
    uiq30FractionalResult = *piq30Coeffs++;

    /* Compute exp^(iq31Fractional). */
    for (ui8Count = _IQ30exp_order; ui8Count > 0; ui8Count--) {
        uiq30FractionalResult = __mpyf_l(iq31Fractional, uiq30FractionalResult);
        uiq30FractionalResult += *piq30Coeffs++;
    }

    /* Scale the iq30 fractional result by to iq31. */
    uiq31FractionalResult = uiq30FractionalResult << 1;

    /*
     * Multiply the integer result in iqN format and the fractional result in
     * iq31 format to obtain the result in iqN format.
     */
    uiqNResult = __mpyf_ul(uiqNIntegerResult, uiq31FractionalResult);

    /*
     * Mark the end of all multiplies. This restores MPY and interrupt states
     * (MSP430 only).
     */
    __mpy_stop(&ui16IntState, &ui16MPYState);

    /* The result is scaled by 2, round the result and scale to iqN format. */
    uiqNResult++;
    uiqNResult >>= 1;

    return uiqNResult;
}
