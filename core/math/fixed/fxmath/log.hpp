#pragma once

#include "port.hpp"
#include "constants.hpp"


namespace ymd::fxmath::details{

struct [[nodiscard]] IqLnIntermediate final{

    using Self = IqLnIntermediate;


    int32_t iq30_result;
    int32_t exp;


    template<size_t Q>
    __attribute__((always_inline,  optimize( "-Ofast" )))
    static constexpr Self from(uint32_t uiqn_input){

        /*
        * Check the sign of the input and for negative saturation for Qs
        * larger than iq26.
        */
        /*
        * Only check the sign of the input and that it is not equal to zero for
        * Qs less than or equal to iq26.
        */

        /* Initialize the exponent value. */
        int32_t exp = (31 - Q);

        /*
        * Scale the input so it is within the following range in iq31:
        *
        *     0.666666 < uiq31_input < 1.333333.
        */
        uint32_t uiq31_input = (uint32_t)uiqn_input;

        // \frac{2}{3}\cdot2^{31}
        constexpr uint32_t IQ31_TWOTHIRD = static_cast<uint32_t>(static_cast<long double>((2.0 / 3) * (1u << 31)));

        while (uiq31_input < uint32_t(IQ31_TWOTHIRD)) {
            uiq31_input <<= 1;
            exp--;
        }

        constexpr uint32_t IQ31_ONE = 0x80000000;
        const int32_t iq31_input = uiq31_input - IQ31_ONE;

        if(iq31_input > INT32_MAX) __builtin_unreachable();

        const int32_t iq32_input = int32_t(iq31_input) * 2;

        int32_t iq30_result = IQ30LOG_COEFFS[0];
        #pragma GCC unroll 8
        for (size_t i = 1; i < LOG_COEFFS_TABLE_SIZE; i++) {
            iq30_result = intrinsics::mul32hss(iq32_input, iq30_result);

            // __builtin_prefetch(IQ30LOG_COEFFS + i, 0, 1);
            iq30_result += IQ30LOG_COEFFS[i];
        }

        return Self{iq30_result, exp};
    }

    template<size_t Q>
    __attribute__((always_inline,  optimize( "-Ofast" )))
    constexpr int32_t into_bits() const{
        int32_t iq_n_result = iq30_result >> (30 - Q);
        /*
        * Add exp * ln(2) to the iqN result. This will never saturate since we
        * check for the minimum value at the start of the function. Negative
        * exponents require seperate handling to allow for an extra bit with the
        * unsigned data type.
        */
        if (exp > 0) {
            iq_n_result += intrinsics::mul32hu(UQ32_LN2, ((int32_t)exp << Q));
        } else {
            iq_n_result -= intrinsics::mul32hu(UQ32_LN2, (((uint32_t) - exp) << Q));
        }

        return iq_n_result;
    }

};



template<size_t Q>
__attribute__((always_inline,  optimize( "-Ofast" )))
constexpr math::fixed<Q, int32_t> ln32u(const math::fixed<Q, uint32_t> x) {
    constexpr uint32_t IQN_MIN = ((Q >= 27) ? fxmath::details::IQNLOG_MIN[Q - 27] : 1);
    const uint32_t uiqn_input = x.to_bits();
    const int32_t ret_bits = [&] -> int32_t{
        if constexpr(Q > 26) {
            if (uiqn_input <= IQN_MIN) {
                return INT32_MIN;
            }
        }
        return fxmath::details::IqLnIntermediate::template from<Q>(uiqn_input)
            .template into_bits<Q>();
    }();

    return math::fixed<Q, int32_t>::from_bits(ret_bits);
}


}

namespace ymd::math{



template<size_t Q>
constexpr 
fixed<Q, int32_t> ln(const fixed<Q, uint32_t> x) {
    return fxmath::details::ln32u<Q>(x);
}

template<size_t Q>
constexpr 
fixed<Q, int32_t> lg(const fixed<Q, uint32_t> x) {
    constexpr double LN10 = 2.30258509299;
    constexpr auto INV_LN10 = fixed<32, uint32_t>(1.0 / LN10);
    return fixed<Q, int32_t>::from_bits(
        intrinsics::mul32hsu(fxmath::details::ln32u<Q>(x).to_bits(), INV_LN10.to_bits()));
}




}