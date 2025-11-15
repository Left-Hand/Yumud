#pragma once

#include <cstdint>
#include <bit>

namespace ymd{

namespace iqmath::details{

template<size_t Q>
[[nodiscard]] static constexpr float _IQNtoF(const int32_t iqNInput){
    uint16_t ui16Exp;
    uint32_t uiq23Result;
    uint32_t uiq31Input;

    /* Initialize exponent to the offset iq value. */
    ui16Exp = 0x3f80 + ((31 - Q) * ((uint32_t) 1 << (23 - 16)));

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


template<size_t Q>
[[nodiscard]] static constexpr int32_t _IQFtoN(const float fv){
    if(std::is_constant_evaluated()){
        return int32_t(fv * int(1 << Q));
    }

    const int32_t d = std::bit_cast<int32_t>(fv);
    const int32_t exponent = ((d >> 23) & 0xff);
    const uint64_t mantissa = (exponent == 0) ? (0) : ((d & ((1 << 23) - 1)) | (1 << 23));

    const uint64_t temp = [&] -> uint64_t {
        if(exponent == 0 or exponent == 0xff){
            return 0;
        }else{
            return LSHIFT(mantissa, exponent - 127);
        }
    }();


    const uint64_t uresult = [&] -> uint64_t {
        static constexpr int s = 23 - Q;
        if constexpr (s == 0){
            return temp;
        }else if constexpr (s > 0){
            return temp >> size_t(s);
        }else{
            return temp << size_t(-s);
        }
    }();
    const int32_t result = d > 0 ? uresult : -uresult;

    if((bool(d > 0) ^ bool(result > 0)) or (uresult > (uint64_t)0x80000000)){//OVERFLOW
        if(d > 0){
            return std::bit_cast<int32_t>(0x7FFFFFFF);
        }else{
            return std::bit_cast<int32_t>(0x80000000);
        }
    }else{
        return std::bit_cast<int32_t>(result);
    }

}
}
}