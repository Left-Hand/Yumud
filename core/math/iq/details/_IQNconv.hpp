#pragma once

#include <cstddef>
#include <cstdint>
#include <bit>

#include <numeric>

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
[[nodiscard]] static constexpr int32_t _IQFtoN(const float fv) {
    static_assert(sizeof(float) == 4);
    constexpr uint32_t NAN_BITS = 0x7fc00000;
    const uint32_t bits = std::bit_cast<uint32_t>(fv);

    if (bits == NAN_BITS) {
        // NaN - 返回0或根据需求处理
        return 0;
    }
    
    const bool is_negative = bool(bits >> 31);
    const int32_t exponent = static_cast<int32_t>((bits >> 23) & 0xFF) - 127;
    const uint32_t mantissa_bits = bits & 0x7FFFFF;
    
    // 处理零和非常小的数
    if (exponent == -127 && mantissa_bits == 0) {
        return 0;  // 零（正或负）
    }
    
    // 检查是否超出IQ表示范围
    if (exponent >= int32_t(Q)) {
        // 溢出 - 返回最大正值或最小负值
        return (is_negative) ? 
            std::numeric_limits<int32_t>::min() : std::numeric_limits<int32_t>::max();
    }
    
    // 构建完整的尾数（包括隐含的1）
    uint64_t mantissa = static_cast<uint64_t>(mantissa_bits) | (1ULL << 23);
    
    // 调整尾数位数，考虑小数点位置
    int32_t shift = 23 - exponent - Q;
    
    int64_t result;
    if (shift >= 0) {
        result = static_cast<int64_t>(mantissa >> shift);
    } else {
        result = static_cast<int64_t>(mantissa << (-shift));
    }
    
    // 应用符号
    if(is_negative) result = -result;
    
    // 检查溢出
    if (result > std::numeric_limits<int32_t>::max()) {
        return std::numeric_limits<int32_t>::max();
    } else if (result < std::numeric_limits<int32_t>::min()) {
        return std::numeric_limits<int32_t>::min();
    }
    
    return static_cast<int32_t>(result);
}



}
}