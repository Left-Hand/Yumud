#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"

namespace ymd::iqmath::details{

/**
 * @brief Computes the exponential of an IQN input.
 *
 * @param iqNInput          IQN type input.
 * @param iqNLookupTable    Integer result lookup table.
 * @param ui8IntegerOffset  Integer portion offset
 * @param iqN_MIN           Minimum parameter value.
 * @param iqN_MAX           Maximum parameter value.
 * @param Q           IQ format.
 *
 *
 * @return                  IQN type result of exponential.
 */

template<const size_t Q>
static constexpr uint32_t __IQNexp(int32_t iqNInput){
    constexpr const uint32_t *iqNLookupTable = _IQNexp_lookup[Q - 1];
    constexpr uint32_t ui8IntegerOffset = _IQNexp_offset[Q - 1];
    constexpr int32_t iqN_MIN = _IQNexp_min[Q - 1];
    constexpr int32_t iqN_MAX = _IQNexp_max[Q - 1];
    
    const uint32_t *piq30Coeffs;
    int32_t iq31Fractional;
    uint32_t uiqNResult;
    uint32_t uiqNIntegerResult;
    [[maybe_unused]] uint32_t uiq30FractionalResult;
    uint32_t uiq31FractionalResult;
    uint32_t ui8Count;

    /* Input is negative. */
    if (iqNInput < 0) {
        /* Check for the minimum value. */
        if (iqNInput < iqN_MIN) {
            return 0;
        }

        /* Extract the fractional portion in iq31 and set sign bit. */
        iq31Fractional = (iqNInput << (31 - Q)) | 0x80000000;

        /* Extract the integer portion. */
        const uint16_t u16Integer = uint16_t((iqNInput >> Q) + 1 + ui8IntegerOffset);

        /* Offset the integer portion and lookup the integer result. */
        uiqNIntegerResult = iqNLookupTable[u16Integer];

        /* Reduce the fractional portion to -ln(2) < iq31Fractional < 0 */
        if (iq31Fractional <= -_iq31_ln2) {
            iq31Fractional += _iq31_ln2;
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
        iq31Fractional = (iqNInput << (31 - Q)) & 0x7fffffff;

        /* Extract the integer portion. */
        const uint16_t u16Integer = uint16_t((iqNInput >> Q) + ui8IntegerOffset);

        /* Offset the integer portion and lookup the integer result. */
        uiqNIntegerResult = iqNLookupTable[u16Integer];

        /* Reduce the fractional portion to 0 < iq31Fractional < ln(2) */
        if (iq31Fractional >= _iq31_ln2) {
            iq31Fractional -= _iq31_ln2;
            uiqNIntegerResult <<= 1;
        }
    }


    /*
     * Initialize the coefficient pointer to the Taylor Series iq30 coefficients
     * for the exponential functions. Set the iq30 result to the first
     * coefficient in the table.
     */
    piq30Coeffs = _IQ30exp_coeffs;

    #if 1
    uiq30FractionalResult = *piq30Coeffs++;

    /* Compute exp^(iq31Fractional). */
    for (ui8Count = IQ30EXP_ORDER; ui8Count > 0; ui8Count--) {
        uiq30FractionalResult = __mpyf_l(iq31Fractional, uiq30FractionalResult);
        uiq30FractionalResult += *piq30Coeffs++;
    }

    /* Scale the iq30 fractional result by to iq31. */
    uiq31FractionalResult = uiq30FractionalResult << 1;
    #else
    uiq31FractionalResult = (*piq30Coeffs++) << 1;
    iq31Fractional <<= 1;
    /* Compute exp^(iq31Fractional). */
    for (ui8Count = _IQ30exp_order; ui8Count > 0; ui8Count--) {
        uiq31FractionalResult = int32_t((int64_t(iq31Fractional) * uint64_t(uiq31FractionalResult)) >> 32);
        uiq31FractionalResult += ((*piq30Coeffs++) << 1);
    }

    #endif



    /*
     * Multiply the integer result in iqN format and the fractional result in
     * iq31 format to obtain the result in iqN format.
     */
    uiqNResult = __mpyf_ul(uiqNIntegerResult, uiq31FractionalResult);


    /* The result is scaled by 2, round the result and scale to iqN format. */
    uiqNResult++;
    uiqNResult >>= 1;

    return uiqNResult;
}

template<const size_t Q>
constexpr math::fixed_t<Q, uint32_t> _IQNexp(math::fixed_t<Q, int32_t> input){
    return math::fixed_t<Q, uint32_t>::from_bits(__IQNexp<Q>((input).to_bits()));
}

}