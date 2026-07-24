#pragma once

#include <cstdint>
#include <bit>

namespace ymd::math::fp16_utils{


static constexpr uint32_t round_shift_right(const uint32_t value, const uint8_t shift) noexcept{
    if(shift == 0) [[unlikely]] return value;
    #if 0
    const uint32_t half = 1UL << (shift - 1);
    const uint32_t mask = (1UL << shift) - 1UL;
    const uint32_t retained = value >> shift;
    const uint32_t dropped = value & mask;
    return retained + (dropped > half || (dropped == half && (retained & 1UL)));
    #else
    const uint32_t rounder = 1 << (shift - 1);
    return (value + rounder) >> shift;
    #endif
}

__attribute__((optimize("-Ofast"), const))
static constexpr uint16_t fp32_to_fp16bits_nonfpu(float value){
    
    #if 0
    
    constexpr uint32_t magic_bits = 15UL << 23;
    constexpr uint32_t f32inf = 255UL << 23;
    constexpr uint32_t f16inf = 31UL << 23;
    constexpr uint32_t round_mask = ~0xFFFU;

    uint32_t in = std::bit_cast<uint32_t>(value);
    uint32_t sign = std::bit_cast<uint32_t>(in) & 0x8000'0000;
    in ^= sign;
    uint16_t out = 0;

    if (in >= f32inf){
        // 特殊值：∞ 或 NaN
        // in >= f32inf 时，in = f32inf（Inf）或 in > f32inf（NaN）
        if (in > f32inf) {
            // NaN 路径
            out = (uint16_t)0x7FFFU;
        } else {
            // ∞ 路径
            out = (uint16_t)0x7C00U;
        }
    } else {
        // 正常范围：使用魔术数字法缩放
        // in < f32inf 保证进入此分支
        in &= round_mask;
        float f = std::bit_cast<float>(in);
        f *= std::bit_cast<float>(magic_bits);
        in = std::bit_cast<uint32_t>(f);
        in -= round_mask;

        if(in > f16inf){
            // 溢出处理
            in = f16inf;
        }
        out = (uint16_t)(in >> 13U);
    }
    out |= (uint16_t)(sign >> 16U);  // 添加符号位
    return out;
    #else

    // https://github.com/Matrixchung/iFOC/src/Common/Math/float16.hpp
    const uint32_t bits = std::bit_cast<uint32_t>(value);
    const uint16_t sign = (uint16_t)((bits >> 16) & 0x8000U);
    const uint32_t exp = (bits >> 23) & 0xFFU;
    const uint32_t mant = bits & 0x7FFFFFU;

    if(exp == 0xFFU)
    {
        if(mant == 0) return sign | 0x7C00U;
        uint16_t payload = (uint16_t)(mant >> 13);
        payload |= 0x0200U; // quiet NaN
        return sign | 0x7C00U | payload;
    }

    if(exp == 0) return sign;

    const int32_t half_exp = (int32_t)exp - 127 + 15;
    if(half_exp >= 0x1F) return sign | 0x7C00U;

    if(half_exp <= 0)
    {
        if(half_exp < -10) return sign;
        const uint32_t mant_with_hidden_bit = mant | 0x800000U;
        const uint32_t rounded = round_shift_right(mant_with_hidden_bit, (uint8_t)(14 - half_exp));
        return sign | (uint16_t)rounded;
    }

    uint32_t half_mant = round_shift_right(mant, 13);
    uint32_t exp_out = (uint32_t)half_exp;
    if(half_mant == 0x0400U)
    {
        half_mant = 0;
        exp_out++;
        if(exp_out >= 0x1FU) return sign | 0x7C00U;
    }

    return sign | (uint16_t)(exp_out << 10) | (uint16_t)half_mant;
    #endif

}


__attribute__((optimize("-Ofast"), const))
static constexpr float fp16bits_to_fp32_nonfpu(uint16_t bits){
    #if 0
    uint32_t sign = bits & 0x8000U;
    uint32_t exp = (bits >> 10U) & 0x1FU;
    uint32_t mant = bits & 0x3FFU;

    uint32_t out_bits = 0;

    if (exp == 0 && mant == 0) {
        // 零值路径：±0
        // exp=0 && mant=0 互斥于后续所有分支
        out_bits = 0;
    } else if (exp == 0) {
        // 非规范化数路径（exp=0, mant≠0）
        // 指数固定为 2^-14，FP32 中 exp32 = 113
        // mant: FP16[10 bits] → FP32[23 bits] = mant << 13
        if (!(mant != 0)) __builtin_unreachable();  // 确保 mant≠0
        uint32_t exp32 = 113U;
        uint32_t mant32 = mant << 13U;
        out_bits = (exp32 << 23U) | mant32;
    } else if (exp == 31U) {
        // 特殊值路径（Inf 或 NaN）
        // 指数为 0xFF，尾数直接映射
        if (exp != 31U) __builtin_unreachable();  // 互斥条件检查
        uint32_t exp32 = 0xFFU;
        uint32_t mant32 = mant << 13U;
        out_bits = (exp32 << 23U) | mant32;
    } else {
        // 规范化数路径（exp ∈ [1, 30]）
        // 指数映射：exp16 + 112 → exp32
        if (exp == 0 || exp == 31U) __builtin_unreachable();  // 前驱条件检查
        uint32_t exp32 = exp + 112U;
        uint32_t mant32 = mant << 13U;
        out_bits = (exp32 << 23U) | mant32;
    }

    out_bits |= (sign << 16U);  // 添加符号位

    #else

    // https://github.com/Matrixchung/iFOC/src/Common/Math/float16.hpp
    const uint32_t sign = ((uint32_t)bits & 0x8000U) << 16;
    uint32_t exp = ((uint32_t)bits >> 10) & 0x1FU;
    uint32_t mant = (uint32_t)bits & 0x03FFU;

    if(exp == 0)
    {
        if(mant == 0) return sign;

        const uint32_t shift = __builtin_clz(mant) - 21U;
        mant <<= shift;
        mant &= 0x03FFU;
        exp = 113U - shift;
        return sign | (exp << 23) | (mant << 13);
    }

    if(exp == 0x1FU) return sign | 0x7F800000U | (mant << 13);

    exp = exp - 15 + 127;
    uint32_t out_bits = sign | (exp << 23) | (mant << 13);
    #endif
    return std::bit_cast<float>(out_bits);
}

[[nodiscard]] constexpr bool fp16bits_is_zero(uint16_t bits) noexcept { 
    return (bits & 0x7FFFU) == 0; }

[[nodiscard]] constexpr bool fp16bits_is_inf(uint16_t bits) noexcept { 
    return (bits & 0x7FFFU) == 0x7C00U; }

[[nodiscard]] constexpr bool fp16bits_is_nan(uint16_t bits) noexcept { 
    return ((bits & 0x7C00U) == 0x7C00U) && ((bits & 0x03FFU) != 0); }

[[nodiscard]] constexpr bool fp16bits_is_finite(uint16_t bits) noexcept { 
    return (bits & 0x7C00U) != 0x7C00U; }


}

