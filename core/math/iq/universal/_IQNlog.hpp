#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


namespace __iqdetails{
/**
 * @brief Computes the base-e logarithm of an IQN input.
 *
 * @param iqNInput          IQN type input.
 * @param iqNMin            Minimum parameter value.
 * @param Q           IQ format.
 *
 * @return                  IQN type result of exponential.
 */

template<size_t Q>
constexpr int32_t __IQNlog(int32_t iqNInput, const int32_t iqNMin)
{
    int16_t i16Exp;
    int32_t iq30Result;
    uint32_t uiq31Input;
    const uint32_t *piq30Coeffs;

    /*
     * Check the sign of the input and for negative saturation for Qs
     * larger than iq26.
     */
    if constexpr(Q > 26) {
        if (iqNInput <= 0) {
            return 0;
        } else if (iqNInput <= iqNMin) {
            return INT32_MIN;
        }
    }
    /*
     * Only check the sign of the input and that it is not equal to zero for
     * Qs less than or equal to iq26.
     */
    else {
        if (iqNInput <= 0) {
            return 0;
        }
    }

    /* Initialize the exponent value. */
    i16Exp = (31 - Q);

    /*
     * Scale the input so it is within the following range in iq31:
     *
     *     0.666666 < uiq31Input < 1.333333.
     */
    uiq31Input = (uint32_t)iqNInput;
    while (uiq31Input < _iq31_twoThird) {
        uiq31Input <<= 1;
        i16Exp--;
    }


    /*
     * Initialize the coefficient pointer to the Taylor Series iq30 coefficients
     * for the logarithm functions. Set the iq30 result to the first
     * coefficient in the table. Subtract one from the iq31 input.
     */
    piq30Coeffs = _IQ30log_coeffs;
    iq30Result = *piq30Coeffs++;
    uiq31Input -= _iq31_one;

    /* Calculate log(uiq31Input) using the iq30 Taylor Series coefficients. */
    for (uint8_t ui8Counter = _IQ30log_order; ui8Counter > 0; ui8Counter--) {
        iq30Result = __mpyf_l(uiq31Input, iq30Result);
        iq30Result += *piq30Coeffs++;
    }


    /*
     * Add i16Exp * ln(2) to the iqN result. This will never saturate since we
     * check for the minimum value at the start of the function. Negative
     * exponents require seperate handling to allow for an extra bit with the
     * unsigned data type.
     */
    if (i16Exp > 0) {
        return iq30Result + __mpyf_ul(_iq31_ln2, ((int32_t)i16Exp << 30));
    } else {
        return iq30Result - __mpyf_ul(_iq31_ln2, (((uint32_t) - i16Exp) << 30));
    }

}

template<size_t Q>
constexpr _iq<30> _IQNlog(_iq<Q> a){
    return _iq<30>::from_i32(__IQNlog<Q>(a.to_i32(), ((Q >= 27) ? _IQNlog_min[Q - 27] : 1)));
}

}