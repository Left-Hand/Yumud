#pragma once

#include "support.hpp"
#include "_IQNtables.hpp"

namespace ymd::fxmath::details{

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


struct alignas(8) [[nodiscard]] ExpIntermediate final{

    uint32_t uiq32Fractional;
    uint32_t uiqNIntegerResult;

    template<size_t Q>
    static constexpr ExpIntermediate from(int32_t iqNInput){
        static_assert(Q <= 30, "out of table range");

        constexpr const uint32_t *iqNLookupTable = _IQNexp_lookup[Q - 1];
        constexpr uint32_t ui8IntegerOffset = _IQNexp_offset[Q - 1];

        uint32_t uiq32Fractional;
        uint32_t uiqNIntegerResult;

        /* Offset the integer portion and lookup the integer result. */
        uiqNIntegerResult = iqNLookupTable[(iqNInput >> Q) + ui8IntegerOffset];

        /* Input is negative. */
        if (iqNInput < 0) {
            /* Extract the fractional portion in iq31 and set sign bit. */
            uiq32Fractional = (iqNInput << (32 - Q));

            /* Reduce the fractional portion to -ln(2) < uiq32Fractional < 0 */
            if (uiq32Fractional >= uint32_t(_uq32_ln2)) {
                uiq32Fractional -= uint32_t(_uq32_ln2);
                uiqNIntegerResult >>= 1;
            }
        }
        /* Input is positive. */
        else {
            /* Extract the fractional portion in iq31 and clear sign bit. */
            uiq32Fractional = ((-iqNInput) << (32 - Q));

            /* Reduce the fractional portion to 0 < uiq32Fractional < ln(2) */
            if (uiq32Fractional >= uint32_t(_uq32_ln2)) {
                uiq32Fractional -= uint32_t(_uq32_ln2);
                uiqNIntegerResult <<= 1;
            }
        }

        // return ExpIntermediate{static_cast<uint32_t>(iq31Fractional << 1), uiqNIntegerResult};
        return ExpIntermediate{uiq32Fractional, uiqNIntegerResult};
    }



    constexpr uint32_t into() const {

        /*
        * Initialize the coefficient pointer to the Taylor Series iq30 coefficients
        * for the exponential functions. Set the iq30 result to the first
        * coefficient in the table.
        */
        const uint32_t *piq31Coeffs = static_cast<const uint32_t *>(IQ31EXP_COEFFS.data());

        uint32_t uiq31FractionalResult = (*piq31Coeffs++);
        /* Compute exp^(uiq31Fractional). */
        for (uint32_t i = 0; i < IQ30EXP_ORDER; i++) {
            uiq31FractionalResult = uint32_t((uint64_t(uiq32Fractional) * uint64_t(uiq31FractionalResult)) >> 32);
            uiq31FractionalResult += (piq31Coeffs[i]);
        }

        /*
        * Multiply the integer result in iqN format and the fractional result in
        * iq31 format to obtain the result in iqN format.
        */
        return uint32_t((uint64_t(uiqNIntegerResult) * uint64_t(uiq31FractionalResult)) >> 32);
    }
};

template<const size_t Q>
constexpr math::fixed<Q, uint32_t> _IQNexp(math::fixed<Q, int32_t> input){
    static_assert(Q <= 30, "out of table range");
    int32_t iqNInput = input.to_bits();
    const uint32_t ret_bits = [&] -> uint32_t{
        if (iqNInput < 0) {
            constexpr int32_t iqN_MIN = _IQNexp_min[Q - 1];
            /* Check for the minimum value. */
            if (iqNInput < iqN_MIN) [[unlikely]] {
                return 0;
            }
        }else{
            constexpr int32_t iqN_MAX = _IQNexp_max[Q - 1];
            /* Check for the maximum value. */
            if (iqNInput > iqN_MAX) [[unlikely]] {
                return UINT32_MAX;
            }
        }
        return ExpIntermediate::from<Q>(iqNInput).into();
    }();

    return math::fixed<Q, uint32_t>::from_bits(ret_bits);
}

}