#pragma once
#include <stdint.h>

#include "support.h"
#include "_IQNtables.hpp"

/**
 * @brief Computes the base-e logarithm of an IQN input.
 *
 * @param iqNInput          IQN type input.
 * @param iqNMin            Minimum parameter value.
 * @param q_value           IQ format.
 *
 * @return                  IQN type result of exponential.
 */
constexpr int_fast32_t __IQNlog(int_fast32_t iqNInput, const int_fast32_t iqNMin, const int8_t q_value)
{
    uint8_t ui8Counter;
    int_fast16_t i16Exp;
    uint_fast16_t ui16IntState;
    uint_fast16_t ui16MPYState;
    int_fast32_t iqNResult;
    int_fast32_t iq30Result;
    uint_fast32_t uiq31Input;
    const uint_fast32_t *piq30Coeffs;

    /*
     * Check the sign of the input and for negative saturation for q_values
     * larger than iq26.
     */
    if (q_value > 26) {
        if (iqNInput <= 0) {
            return 0;
        } else if (iqNInput <= iqNMin) {
            return INT32_MIN;
        }
    }
    /*
     * Only check the sign of the input and that it is not equal to zero for
     * q_values less than or equal to iq26.
     */
    else {
        if (iqNInput <= 0) {
            return 0;
        }
    }

    /* Initialize the exponent value. */
    i16Exp = (31 - q_value);

    /*
     * Scale the input so it is within the following range in iq31:
     *
     *     0.666666 < uiq31Input < 1.333333.
     */
    uiq31Input = (uint_fast32_t)iqNInput;
    while (uiq31Input < _iq31_twoThird) {
        uiq31Input <<= 1;
        i16Exp--;
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
     * for the logarithm functions. Set the iq30 result to the first
     * coefficient in the table. Subtract one from the iq31 input.
     */
    piq30Coeffs = _IQ30log_coeffs;
    iq30Result = *piq30Coeffs++;
    uiq31Input -= _iq31_one;

    /* Calculate log(uiq31Input) using the iq30 Taylor Series coefficients. */
    for (ui8Counter = _IQ30log_order; ui8Counter > 0; ui8Counter--) {
        iq30Result = __mpyf_l(uiq31Input, iq30Result);
        iq30Result += *piq30Coeffs++;
    }

    /* Scale the iq30 result to match the function iq type. */
    iqNResult = iq30Result >> (30 - q_value);

    /*
     * Add i16Exp * ln(2) to the iqN result. This will never saturate since we
     * check for the minimum value at the start of the function. Negative
     * exponents require seperate handling to allow for an extra bit with the
     * unsigned data type.
     */
    if (i16Exp > 0) {
        iqNResult += __mpyf_ul(_iq31_ln2, ((int_fast32_t)i16Exp << q_value));
    } else {
        iqNResult -= __mpyf_ul(_iq31_ln2, (((uint_fast32_t) - i16Exp) << q_value));
    }

    /*
     * Mark the end of all multiplies. This restores MPY and interrupt states
     * (MSP430 only).
     */
    __mpy_stop(&ui16IntState, &ui16MPYState);

    return iqNResult;
}

template<int8_t q_value>
constexpr int32_t _IQNlog(int32_t a)
{
    return __IQNlog(a, q_value >= 27 ? _IQNlog_min[q_value - 27] : 1, q_value);
}
