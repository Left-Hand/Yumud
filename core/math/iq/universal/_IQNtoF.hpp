#pragma once

#include "support.hpp"

namespace __iqdetails{

template<const size_t Q>
constexpr float _IQNtoF(_iq<Q> iqNInput)
{
    uint16_t ui16Exp;
    uint32_t uiq23Result;
    uint32_t uiq31Input;

    /* Initialize exponent to the offset iq value. */
    ui16Exp = 0x3f80 + ((31 - Q) * ((uint32_t) 1 << (23 - 16)));

    /* Save the sign of the iqN input to the exponent construction. */
    if (iqNInput.signbit()) {
        ui16Exp |= 0x8000;
        uiq31Input = -iqNInput.to_u32();
    } else if (iqNInput.to_i32() == 0) {
        return (0);
    } else {
        uiq31Input = iqNInput.to_u32();
    }

    /* Scale the iqN input to uiq31 by keeping track of the exponent. */
    while ((uint16_t)(uiq31Input >> 16) < 0x8000) {
        uiq31Input <<= 1;
        ui16Exp -= 0x0080;
    }

    /* Round the uiq31 result and and shift to uiq23 */
    uiq23Result = (uiq31Input + 0x0080) >> 8;

    /* Remove the implied MSB bit of the mantissa. */
    uiq23Result &= ~0x00800000;

    /*
     * Add the constructed exponent and sign bit to the mantissa. We must use
     * an add in the case where rounding would cause the mantissa to overflow.
     * When this happens the mantissa result is two where the MSB is zero and
     * the LSB of the exp is set to 1 instead. Adding one to the exponent is the
     * correct handling for a mantissa of two. It is not required to scale the
     * mantissa since it will always be equal to zero in this scenario.
     */
    uiq23Result += (uint32_t) ui16Exp << 16;

    /* Return the mantissa + exp + sign result as a floating point type. */
    return std::bit_cast<float>(uiq23Result);
}

}