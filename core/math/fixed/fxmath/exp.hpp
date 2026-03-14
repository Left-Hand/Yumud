#pragma once

#include "port.hpp"
#include "constants.hpp"

namespace ymd::fxmath{
    
namespace details{

struct alignas(8) [[nodiscard]] ExpIntermediate final{

    uint32_t uiq32_fractional;
    uint32_t uiqn_integer_result;

    template<size_t Q>
    __attribute__((always_inline,  optimize( "-Ofast" )))
    static constexpr ExpIntermediate from(int32_t iqn_input){
        static_assert(Q <= 30, "out of table range");

        constexpr const auto & iqNLookupTable = _IQNexp_lookup[Q - 1];
        constexpr uint32_t ui8_integer_offset = _IQNexp_offset[Q - 1];

        uint32_t uiq32_fractional;
        uint32_t uiqn_integer_result;

        /* Offset the integer portion and lookup the integer result. */
        uiqn_integer_result = iqNLookupTable[(iqn_input >> Q) + ui8_integer_offset];

        /* Input is negative. */
        if (iqn_input < 0) {
            /* Extract the fractional portion in iq31 and set sign bit. */
            uiq32_fractional = (iqn_input << (32 - Q));

            /* Reduce the fractional portion to -ln(2) < uiq32_fractional < 0 */
            if (uiq32_fractional >= uint32_t(UQ32_LN2)) {
                uiq32_fractional -= uint32_t(UQ32_LN2);
                uiqn_integer_result >>= 1;
            }
        }
        /* Input is positive. */
        else {
            /* Extract the fractional portion in iq31 and clear sign bit. */
            uiq32_fractional = ((-iqn_input) << (32 - Q));

            /* Reduce the fractional portion to 0 < uiq32_fractional < ln(2) */
            if (uiq32_fractional >= uint32_t(UQ32_LN2)) {
                uiq32_fractional -= uint32_t(UQ32_LN2);
                uiqn_integer_result <<= 1;
            }
        }

        // return ExpIntermediate{static_cast<uint32_t>(iq31Fractional << 1), uiqn_integer_result};
        return ExpIntermediate{uiq32_fractional, uiqn_integer_result};
    }


    __attribute__((always_inline,  optimize( "-Ofast" )))
    [[nodiscard]] constexpr uint32_t into_bits() const {

        /*
        * Initialize the coefficient pointer to the Taylor Series iq30 coefficients
        * for the exponential functions. Set the iq30 result to the first
        * coefficient in the table.
        */
        const uint32_t *piq31_coeffs = static_cast<const uint32_t *>(IQ31EXP_COEFFS.data());

        uint32_t uiq31_fractional_result = (*piq31_coeffs++);
        /* Compute exp^(uiq31Fractional). */
        for (size_t i = 0; i < EXP_COEFFS_TABLE_SIZE; i++) {
            uiq31_fractional_result = intrinsics::mul32hu(uiq32_fractional, uiq31_fractional_result);
            uiq31_fractional_result += (piq31_coeffs[i]);
        }

        /*
        * Multiply the integer result in iqN format and the fractional result in
        * iq31 format to obtain the result in iqN format.
        */
        return intrinsics::mul32hu(uiqn_integer_result, uiq31_fractional_result);
    }
};


}
}

namespace ymd::math{
template<size_t Q>
constexpr math::fixed<Q, uint32_t> exp(const math::fixed<Q, int32_t> x) {
    static_assert(Q <= 30, "out of table range");
    int32_t iqn_input = x.to_bits();


    if (iqn_input < 0) {
        constexpr int32_t iqN_MIN = fxmath::details::_IQNexp_min[Q - 1];
        /* Check for the minimum value. */
        if (iqn_input < iqN_MIN) [[unlikely]] {
            return math::fixed<Q, uint32_t>::from_bits(0);
        }
    }else{
        constexpr int32_t iqN_MAX = fxmath::details::_IQNexp_max[Q - 1];
        /* Check for the maximum value. */
        if (iqn_input > iqN_MAX) [[unlikely]] {
            return math::fixed<Q, uint32_t>::from_bits(UINT32_MAX);
        }
    }
    return math::fixed<Q, uint32_t>::from_bits(fxmath::details::ExpIntermediate::from<Q>(iqn_input).into_bits());

}

}