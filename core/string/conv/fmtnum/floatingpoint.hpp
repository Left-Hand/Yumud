#pragma once

#include "decimal.hpp"
#include "fixedpoint.hpp"
#include <tuple>

namespace ymd::str{
// 计算正值浮点数的整数部分 
// 此代码不使用任何浮点运算，nan/inf值安全由调用者保障
static constexpr uint32_t floor_abs_f32_nonfpu(const float f_val){
    // Bitcast float to uint32_t without violating strict aliasing
    uint32_t u = std::bit_cast<uint32_t>(f_val);

    const uint32_t exp  = (u >> 23) & 0xFFU;
    const uint32_t mant = u & 0x7FFFFFU;

    // Caller guarantees positive, finite, so sign==0 and exp != 0xFF
    // Handle zero and denormals: integer part is 0
    if (exp == 0) {
        return 0U;
    }

    const int32_t unbiased_exp = static_cast<int32_t>(exp) - 127;
    // If value < 1.0, integer part is 0
    if (unbiased_exp < 0) {
        return 0U;
    }

    // For normalized numbers: value = (1 + mant / 2^23) * 2^(unbiased_exp)
    // We want floor(value), which is an integer.

    // The number has (unbiased_exp + 1) bits in integer part (since 1.xxxx * 2^e)
    // Total significant bits: 24 (implicit 1 + 23 mantissa)

    if (unbiased_exp >= 24) {
        // Integer part has more than 24 bits → mantissa doesn't affect lower bits
        // Result is 1 << unbiased_exp, but may overflow uint32_t
        // However, max float < 2^128, but uint32_t max is ~4e9 (~2^32)
        // So if unbiased_exp >= 32, result overflows → but caller must ensure representable?
        // Since return type is uint32_t, we assume input integer part fits in uint32_t.
        if (unbiased_exp >= 32) {
            // Technically undefined, but we return 0 or saturate? 
            // Per problem: assume valid input that fits.
            // Let's assume input is such that integer part <= UINT32_MAX.
            // So we require unbiased_exp < 32.
            // If not, behavior undefined — but we'll shift anyway (C++ UB for >= width)
            // To avoid UB, clamp:
            return 0xFFFFFFFFU; // or 0? Better to assume valid input.
        }
        return (1U << static_cast<uint32_t>(unbiased_exp));
    }

    // Construct the full 24-bit significand: 1.mant => (1 << 23) | mant
    const uint32_t sig = (1U << 23) | mant;

    // Shift right by (23 - unbiased_exp) to get integer part
    // Because: sig * 2^(unbiased_exp - 23) = integer when unbiased_exp >=0 and <24
    const uint32_t shift = 23 - static_cast<uint32_t>(unbiased_exp);
    return sig >> shift;
}


static constexpr uint32_t frac_abs_f32_nonfpu(const float f_val, const uint32_t scale) {
    const uint32_t u = std::bit_cast<uint32_t>(f_val);
    const int32_t exp = ((u >> 23) & 0xFF) - 127;
    const uint32_t mant = u & 0x7FFFFFU;

    // Handle zero
    if (u == 0) return 0;

    // Caller guarantees positive and finite
    if (exp < 0) {
        // Value < 1.0
        const uint32_t sig = (1U << 23) | mant;
        const int32_t shift = 23 - exp; // > 23
        // sig * scale <= (2^24) * scale
        // If shift is large enough that sig * scale < 2^(shift-1), result is 0
        // Max sig*scale: assume scale <= 100000 (10^5) → ~1.6e12 < 2^41
        if (shift > 60) {
            return 0;
        }
        const uint64_t num = static_cast<uint64_t>(sig) * scale;
        const uint32_t res = (static_cast<uint32_t>((num) >> (shift - 1)) + 1) >> 1;
        return res;
    }

    if (exp >= 23) {
        return 0;
    }

    const uint32_t sig = (1U << 23) | mant;
    const uint32_t frac_bits = sig & ((1U << (23 - exp)) - 1);
    const uint32_t shift = 23 - exp; // 1 to 23
    const uint64_t num = static_cast<uint64_t>(frac_bits) * scale;
    const uint32_t res = (static_cast<uint32_t>((num) >> (shift - 1)) + 1) >> 1;
    return res;
}

// nan/inf值安全由调用者保障
static constexpr uint32_t floor_abs_f32(const float f_val){
    #if 0
    auto hwfp_version = [&]{
        return static_cast<uint32_t>(f_val);
    };


    #ifdef __FPU_PRESENT
        return hwfp_version();
    #endif

    if(std::is_constant_evaluated()){
        return hwfp_version();
    }

    #endif

    return floor_abs_f32_nonfpu(f_val);
}


// 计算正值浮点数的小数部分 例12.34当scale为1000时返回340
// nan/inf值安全由调用者保障
static constexpr uint32_t frac_abs_f32(const float f_val, const uint32_t scale){

    #if 0
    auto hwfp_version = [&]{
        return static_cast<uint32_t>((f_val - floor_abs_f32(f_val)) * scale + 0.5f);
    };


    #ifdef __FPU_PRESENT
        return hwfp_version();
    #endif

    if(std::is_constant_evaluated()){
        return hwfp_version();
    }

    #endif

    return frac_abs_f32_nonfpu(f_val, scale);
}

static constexpr std::pair<uint32_t, uint32_t> u64_div_u32(const uint64_t a, const uint32_t b){
    // TODO 避免64位除法
    // 对于较大的值，使用快速分割算法
    // 避免直接的64位除法，使用查找表或近似算法
    
    // 使用二分查找或预计算的倒数来避免除法
    // 基于scale的值（来自pow10_table）我们可以使用专门的快速算法
    
    // 通用快速除法，针对pow10_table中的值进行优化
    return {
        static_cast<uint32_t>(a / b),
        static_cast<uint32_t>(a % b)
    };
}

// 将正值浮点数拆分为(u32, u32)对，digit_part为整数部分，frac_part为小数部分
// 此代码不使用任何浮点运算，nan/inf值安全由调用者保障
// 例：在给定precision为4时114.514返回(114,5140)
// static_assert(depart_abs_f32(114.5140, 4).digit_part == 114);
// static_assert(depart_abs_f32(114.5140, 4).frac_part == 5140);
[[maybe_unused]] static constexpr DigitFracPair depart_abs_f32(
    float fval, 
    uint8_t precision
) {
    constexpr size_t MAX_PRECSION = std::size(POW10_TABLE) - 1;
    if (precision > MAX_PRECSION) precision = MAX_PRECSION;

    // 计算精度缩放因子
    const uint32_t scale = POW10_TABLE[precision];

    uint32_t digit_part = floor_abs_f32(fval);
    uint32_t frac_part = frac_abs_f32(fval, scale);

    if(frac_part >= scale){
        digit_part += 1;
        frac_part -= scale;
    }

    return {
        .digit_part = digit_part,
        .frac_part = frac_part
    };

}


[[maybe_unused]] static constexpr char * _fmtnum_f32(
    char* p_str, 
    float value, 
    uint8_t precision
) {
    constexpr size_t MAX_PRECSION = std::size(POW10_TABLE) - 1;
    if (precision > MAX_PRECSION) precision = MAX_PRECSION;
    
    
    const bool is_negative = (std::bit_cast<uint32_t>(value) >> 31) != 0;

    const uint32_t unsigned_bits = std::bit_cast<uint32_t>(value) & 0x7FFFFFFF;
    
    // 检查NaN
    if ((unsigned_bits) > uint32_t(0x7F800000)) [[unlikely]] {
        p_str[0] = 'n'; p_str[1] = 'a'; p_str[2] = 'n';
        return p_str + 3;
    }
    
    if (is_negative) {
        p_str[0] = '-';
        p_str++;
    }

    // 检查无穷大
    if ((unsigned_bits) == uint32_t(0x7F800000)) [[unlikely]] {
        p_str[0] = 'i'; p_str[1] = 'n'; p_str[2] = 'f';
        return p_str + 3;
    }

    const int32_t exponent = ((unsigned_bits >> 23) & 0xFF) - 127;
    const uint32_t mantissa = unsigned_bits & 0x7FFFFF;
    
    const auto parts = [&] -> DigitFracPair{
        // 处理零
        if (exponent == -127 && mantissa == 0) [[unlikely]] {
            return {
                .digit_part = 0,
                .frac_part = 0
            };
        }else{
            return depart_abs_f32(std::bit_cast<float>(unsigned_bits), precision);
        }
    }();


    p_str = parts.fmt_str(p_str, precision);
    return p_str;
}


}