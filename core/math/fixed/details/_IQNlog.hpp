#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"


namespace ymd::fxmath::details{


template<size_t Q>
constexpr int32_t __IQNln(uint32_t iqNInput)
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
constexpr math::fixed<Q, int32_t> _IQNln(math::fixed<Q, uint32_t> a){
    const uint32_t iqNMin = ((Q >= 27) ? IQNLOG_MIN[Q - 27] : 1);
    const uint32_t iqNInput = a.to_bits();
    const int32_t ret_bits = [&] -> int32_t{
        if constexpr(Q > 26) {
            if (iqNInput <= iqNMin) {
                return INT32_MIN;
            }
        }
        return __IQNln<Q>(iqNInput);
    }();

    return math::fixed<Q, int32_t>::from_bits(ret_bits);
}

}

namespace ymd::math{

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed<Q, int32_t> lg(const fixed<Q, uint32_t> x) {
    constexpr double LN10 = 2.30258509299;
    constexpr auto INV_LN10 = fixed<32, uint32_t>(1.0 / LN10);
    return fixed<Q, int32_t>(fxmath::details::_IQNln(x)) * INV_LN10;
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed<Q, int32_t> ln(const fixed<Q, uint32_t> x) {
    return fixed<Q, int32_t>(fxmath::details::_IQNln(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed<Q, uint32_t> exp(const fixed<Q, int32_t> x) {
    return fixed<Q, uint32_t>(fxmath::details::_IQNexp<Q>(x));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed<Q, uint32_t> pow(const fixed<Q, uint32_t> base, const fixed<Q, int32_t> exponent) {
    return exp(exponent * ln(base));
}

template<size_t Q>
__attribute__((always_inline)) constexpr 
fixed<Q, uint32_t> pow(const fixed<Q, uint32_t> base, const int32_t times) {
    //TODO 判断使用循环还是pow运算 选取最优时间
    return exp(times * ln(base));
}

}