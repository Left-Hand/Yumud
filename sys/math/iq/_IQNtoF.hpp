#pragma once

#include <stdint.h>

#include "support.h"

/**
 * @brief Converts IQN type to floating point.
 *
 * @param iqNInput        IQN type value input to be converted.
 * @param q_value         IQ format.
 *
 * @return                Conversion of iqNInput to floating point.
 */
#if defined(__TI_COMPILER_VERSION__)
#pragma FUNC_ALWAYS_INLINE(__IQNtoF)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma inline = forced
#endif


template<const int8_t q_value>
constexpr float _IQNtoF(int_fast32_t iqNInput)
{
    uint_fast16_t ui16Exp;
    uint_fast32_t uiq23Result;
    uint_fast32_t uiq31Input;

    /* Initialize exponent to the offset iq value. */
    ui16Exp = 0x3f80 + ((31 - q_value) * ((uint_fast32_t) 1 << (23 - 16)));

    /* Save the sign of the iqN input to the exponent construction. */
    if (iqNInput < 0) {
        ui16Exp |= 0x8000;
        uiq31Input = -iqNInput;
    } else if (iqNInput == 0) {
        return (0);
    } else {
        uiq31Input = iqNInput;
    }

    /* Scale the iqN input to uiq31 by keeping track of the exponent. */
    while ((uint_fast16_t)(uiq31Input >> 16) < 0x8000) {
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
    uiq23Result += (uint_fast32_t) ui16Exp << 16;

    /* Return the mantissa + exp + sign result as a floating point type. */
    return std::bit_cast<float>(uiq23Result);
}