#pragma once

#include <cstddef>
#include <cstdint>
#include <bit>

#include <numeric>

namespace ymd{

namespace fxmath::details{

#if 1

[[nodiscard]] static constexpr float _IQNtoF(const int32_t iqNInput, size_t Q){
    //这段代码只有一处使用了Q, 不需要使用模板
    uint32_t uiq23_result_bits;
    uint32_t uiq31Input;

    /* Initialize exponent to the offset iq value. */
    uint32_t ui16Exp = static_cast<uint32_t>(0x3f80 + ((31u - Q) * (0x80)));

    /* Save the sign of the iqN input to the exponent construction. */
    if (iqNInput < 0) {
        ui16Exp |= 0x8000;
        if (iqNInput == std::numeric_limits<int32_t>::min()) [[unlikely]] {
            uiq31Input = 0x80000000;
        } else {
            uiq31Input = -iqNInput;
        }
    } else if (iqNInput == 0) {
        return (0);
    } else {
        uiq31Input = iqNInput;
    }

    const size_t leading_zeros = __builtin_clz(uiq31Input);
    uiq31Input <<= leading_zeros;
    ui16Exp -= (leading_zeros << 7);

    /* Right shift to uiq23 */
    uiq23_result_bits = uiq31Input >> 8;

    /* Remove the implied MSB bit of the mantissa. */
    uiq23_result_bits &= ~0x00800000;


    if (uiq23_result_bits == 0 && uiq31Input != 0) {
        // 尾数为0但值不为0，这意味着尾数是1.0
        // 指数已经正确，不需要调整
        // 但需要确保浮点数构造正确
    }

    /* Add the constructed exponent and sign bit to the mantissa. */
    uiq23_result_bits += ui16Exp << 16;

    /* Return as float. */
    return std::bit_cast<float>(uiq23_result_bits);
}
#else


#endif



[[nodiscard]] static constexpr int32_t _IQFtoN(const float fv, size_t Q){
    //这段代码没有使用Q进行特化，可以动态输入
    static_assert(sizeof(float) == 4);
    
    // IEEE 754 浮点数常量定义
    constexpr uint32_t SIGN_MASK = 0x80000000U;      // 符号位掩码
    constexpr uint32_t EXPONENT_MASK = 0x7F800000U;  // 指数位掩码
    constexpr uint32_t MANTISSA_MASK = 0x007FFFFFU;  // 尾数位掩码
    constexpr uint32_t EXPONENT_BIAS = 127;          // 指数偏移值
    
    const uint32_t bits = std::bit_cast<uint32_t>(fv);
    uint32_t mantissa_bits = (bits & MANTISSA_MASK);
    const bool is_negative = (bits & SIGN_MASK) != 0;

    // 检测 NaN: 指数全为1且尾数非零
    if ((bits & EXPONENT_MASK) == EXPONENT_MASK && (mantissa_bits) != 0) [[unlikely]]{
        // NaN 输入：返回零作为默认处理
        // 维护说明: 根据IQ数学库设计原则，NaN被视为无效输入，返回零值
        return 0;
    }

    const int32_t exponent = static_cast<int32_t>((bits & EXPONENT_MASK) >> 23) - static_cast<int32_t>(EXPONENT_BIAS);
    mantissa_bits |= (1u << 23);

    
    // 检查数值是否超出IQ格式表示范围（过大）
    if (exponent >= static_cast<int32_t>(Q)) [[unlikely]]{
        // 溢出情况：输入数值大于IQ格式最大可表示值
        // 维护说明: 防止转换结果超出int32_t范围导致未定义行为
        return is_negative ? 
            std::numeric_limits<int32_t>::min() : 
            std::numeric_limits<int32_t>::max();
    }
    
    // 检查数值是否过小（下溢）
    if (exponent < -(static_cast<int32_t>(Q) + 32)) [[unlikely]]{

        // 处理非规格化数: 指数全为0，表示极小数
        if (exponent == -127) [[unlikely]]{
            // 非规格化数输入：返回零
            // 维护说明: 非规格化数表示的数值远小于IQ格式精度，归零处理
            return 0;
        }
        // 下溢情况：输入数值小于IQ格式最小可表示精度
        // 维护说明: 避免大量右移操作产生无意义结果，直接返回零
        return 0;
    }
    
    // 计算位移量：调整尾数以适应Q格式
    const int32_t right_shift = 23 - exponent - static_cast<int32_t>(Q);


    uint32_t unsigned_result;
    
    if (right_shift >= 0) {
        if(right_shift >= 32) [[unlikely]]{
            return 0;
        }else{
            // 右移操作：降低精度以适配Q格式
            unsigned_result = mantissa_bits >> right_shift;
        }
    } else {
        // 左移操作：提高精度以适配Q格式
        const uint32_t left_shift = static_cast<uint32_t>(-right_shift);

        // 检查是否需要过度左移（可能导致溢出）
        if (left_shift > 31) [[unlikely]]{
            // 过度左移情况：结果超出int32_t可表示范围
            // 维护说明: 防止左移操作导致未定义行为，返回边界值
            return is_negative ? 
                std::numeric_limits<int32_t>::min() : 
                std::numeric_limits<int32_t>::max();
        }
        
        const uint32_t overflow_threshold = (std::numeric_limits<uint32_t>::max() >> left_shift);
        // 检查左移是否会导致溢出
        if (mantissa_bits > overflow_threshold) [[unlikely]]{
            // 溢出情况：左移操作超出uint32_t范围
            // 维护说明: 防止位运算溢出，返回IQ格式边界值
            return is_negative ? 
                std::numeric_limits<int32_t>::min() : 
                std::numeric_limits<int32_t>::max();
        }
        
        unsigned_result = mantissa_bits << left_shift;
    }
    
    if (is_negative) {
        // 处理负数：检查绝对值是否超出范围
        if (unsigned_result >= std::bit_cast<uint32_t>(std::numeric_limits<int32_t>::min())) [[unlikely]] {
            // 绝对值超出负数范围边界
            // 维护说明: 防止负数转换时溢出或INT_MIN边界情况
            return std::numeric_limits<int32_t>::min();
        }
        return -static_cast<int32_t>(unsigned_result);
    } else {
        // 处理正数：检查是否超出范围
        if (unsigned_result > std::bit_cast<uint32_t>(std::numeric_limits<int32_t>::max())) [[unlikely]] {
            // 正数超出范围
            // 维护说明: 防止正数转换时溢出
            return std::numeric_limits<int32_t>::max();
        }
        return static_cast<int32_t>(unsigned_result);
    }
}




}
}