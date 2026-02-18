#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


namespace ymd::fxmath::details{
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
constexpr int32_t __IQNlog(uint32_t iqNInput, const int32_t iqNMin)
{
    int32_t i16Exp;
    int32_t iq30Result;
    uint32_t uiq31Input;
    const uint32_t *piq30Coeffs = IQ30LOG_COEFFS;

    /*
     * Check the sign of the input and for negative saturation for Qs
     * larger than iq26.
     */
    /*
     * Only check the sign of the input and that it is not equal to zero for
     * Qs less than or equal to iq26.
     */
    if constexpr(Q > 26) {
        if (iqNInput <= iqNMin) {
            return INT32_MIN;
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
    while (uiq31Input < uint32_t(_iq31_twoThird)) {
        uiq31Input <<= 1;
        i16Exp--;
    }


    /*
     * Initialize the coefficient pointer to the Taylor Series iq30 coefficients
     * for the logarithm functions. Set the iq30 result to the first
     * coefficient in the table. Subtract one from the iq31 input.
     */
    iq30Result = *piq30Coeffs++;


    const int32_t iq31Input = uiq31Input - _iq31_one;
    if(iq31Input > INT32_MAX) __builtin_unreachable();
    const int32_t iq32Input = int32_t(iq31Input) * 2;
    for (size_t i = 0; i < IQ30LOG_ORDER; i++) {
        iq30Result = static_cast<int32_t>((static_cast<int64_t>(iq32Input) * iq30Result) >> 32);
        iq30Result += piq30Coeffs[i];
    }

    int32_t iqNResult = iq30Result >> (30 - Q);
    /*
     * Add i16Exp * ln(2) to the iqN result. This will never saturate since we
     * check for the minimum value at the start of the function. Negative
     * exponents require seperate handling to allow for an extra bit with the
     * unsigned data type.
     */
    if (i16Exp > 0) {
        iqNResult += uint32_t(uint64_t(_uq32_ln2) * uint32_t(((int32_t)i16Exp << Q)) >> 32) ;
    } else {
        iqNResult -= uint32_t(uint64_t(_uq32_ln2) * uint32_t((((uint32_t) - i16Exp) << Q)) >> 32) ;
    }

    return iqNResult;
}

template<size_t Q>
constexpr math::fixed<Q, int32_t> _IQNlog(math::fixed<Q, uint32_t> a){
    return math::fixed<Q, int32_t>::from_bits(__IQNlog<Q>(a.to_bits(), ((Q >= 27) ? IQNLOG_MIN[Q - 27] : 1)));
}

}