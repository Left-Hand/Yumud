#pragma once

#include "decimal.hpp"


namespace ymd::str{


struct [[nodiscard]] alignas(size_t) DigitFracPair32 final{
    uint32_t digit_part;
    uint32_t frac_part;


    __attribute__((always_inline))
    [[nodiscard]] constexpr char * fmt_str(char * __restrict p_str, const uint8_t precision) const noexcept {
        auto & self = *this;
        // 转换整数部分
        p_str = _fmtnum_u32_dec_fittest(p_str, self.digit_part);
        
        // 转换小数部分
        if (precision > 0) {
            p_str[0] = '.';
            p_str++;
            _fmtnum_u32_dec_padded({p_str, p_str + precision} , self.frac_part);
            p_str += precision;
        }

        return p_str;
    }
};



// 对32位定点数的绝对值求得整数部分和小数部分
// 0 < q_num <= 32
__attribute__((optimize("Ofast")))
[[nodiscard]] static constexpr DigitFracPair32 depart_abs_fixedpoint32(
    uint32_t abs_value_bits, 
    uint8_t precision,
    uint8_t q_num
){

    if(q_num > 32) __builtin_unreachable();
    if(q_num == 0) __builtin_unreachable();

    if(q_num == 32) [[unlikely]]{
        q_num = 31;
        abs_value_bits >>= 1;
    }
    
    uint32_t digit_part = (uint32_t(abs_value_bits) >> q_num);
    
    uint32_t frac_part;

    constexpr size_t MAX_PRECSION = POW10_TABLE.size() - 1;
    if(precision > MAX_PRECSION) __builtin_unreachable();

    const uint32_t pow10_scale = POW10_TABLE[precision];
    const uint32_t lower_mask = (1u << q_num) - 1;
    
    // 使用64位整数进行计算，避免溢出
    const uint64_t frac_scaled_bits = static_cast<uint64_t>(abs_value_bits & lower_mask) * pow10_scale;

    frac_part = (static_cast<uint32_t>(frac_scaled_bits >> (q_num - 1)) + 1) >> 1;

    // 检查是否需要进位到整数部分
    if(frac_part >= pow10_scale){
        digit_part += 1;
    
        // 如果发生进位，调整小数部分
        frac_part -= pow10_scale;
    }

    return {
        .digit_part = digit_part, 
        .frac_part = frac_part
    };
}


// 0 <= q_num <= 32
[[nodiscard]] static constexpr char * _fmtnum_abs_fixedpoint32(
    char * __restrict p_str, 
    uint32_t abs_value_bits, 
    uint8_t precision, 
    uint8_t q_num
){
    // 安全限制precision，确保不超出表格范围
    constexpr size_t MAX_PRECSION = POW10_TABLE.size() - 1;
    if(precision > MAX_PRECSION) precision = MAX_PRECSION;

    const auto parts = [&] -> DigitFracPair32{
        if(q_num == 0)[[unlikely]]{
            return {abs_value_bits, 0};
        }
        return depart_abs_fixedpoint32(abs_value_bits, precision, q_num);
    }();

    p_str = parts.fmt_str(p_str, precision);
    return p_str;
}


// ==================== 64位定点数拆分 ====================

struct [[nodiscard]] alignas(16) DigitFracPair64 final {
    uint64_t digit_part;
    uint32_t frac_part;

    __attribute__((always_inline))
    [[nodiscard]] constexpr char * fmt_str(char * __restrict p_str, const uint8_t precision) const noexcept {
        auto & self = *this;


        if(self.digit_part <= UINT32_MAX) [[likely]] {
            p_str = _fmtnum_u32_dec_fittest(p_str, static_cast<uint32_t>(self.digit_part));
        } else {
            p_str = _fmtnum_u64_dec_fittest(p_str, self.digit_part);
        }

        // 转换小数部分
        if(precision > 0) {
            p_str[0] = '.';
            p_str++;
            _fmtnum_u32_dec_padded({p_str, p_str + precision}, self.frac_part);
            p_str += precision;
        }

        return p_str;
    }
};


// 对64位定点数的绝对值求得整数部分和小数部分
// 不使用128位乘除法，不使用64位除法
// 0 < q_num <= 64
__attribute__((optimize("Ofast")))
[[nodiscard]] static constexpr DigitFracPair64 depart_abs_fixedpoint64(
    uint64_t abs_value_bits,
    uint8_t precision,
    uint8_t q_num
) {

    if(q_num > 64) __builtin_unreachable();
    if(q_num == 0) __builtin_unreachable();

    if(q_num == 64) [[unlikely]] {
        q_num = 63;
        abs_value_bits >>= 1;
    }

    uint64_t digit_part = abs_value_bits >> q_num;

    constexpr size_t MAX_PRECISION = POW10_TABLE.size() - 1;
    if(precision > MAX_PRECISION) __builtin_unreachable();

    const uint32_t pow10_scale = POW10_TABLE[precision];
    const uint64_t lower_mask = (1ull << q_num) - 1;
    const uint64_t frac_bits = abs_value_bits & lower_mask;

    // ---- 将64位分数部分拆为两个32位半区，分别乘以pow10_scale ----
    // frac_bits = frac_hi32 * 2^32 + frac_lo32
    // prod = frac_hi32 * pow10_scale * 2^32 + frac_lo32 * pow10_scale
    const uint32_t frac_lo32 = static_cast<uint32_t>(frac_bits);
    const uint32_t frac_hi32 = static_cast<uint32_t>(frac_bits >> 32);

    // 32×32→64 乘法（允许），每个乘积 ≤ (2^32-1)*(10^9-1) < 2^62
    const uint64_t prod_lo = static_cast<uint64_t>(frac_lo32) * pow10_scale;
    const uint64_t prod_hi = static_cast<uint64_t>(frac_hi32) * pow10_scale;

    // ---- 将两个64位乘积重组为96位中间结果 [W2:W1:W0] ----
    const uint32_t W0 = static_cast<uint32_t>(prod_lo);                     // bits [31:0]
    const uint32_t prod_lo_hi = static_cast<uint32_t>(prod_lo >> 32);       // bits [63:32] of prod_lo
    const uint32_t prod_hi_lo = static_cast<uint32_t>(prod_hi);              // bits [31:0] of prod_hi

    // W1 = prod_lo[63:32] + prod_hi[31:0]，需要处理进位
    const uint64_t W1_temp = static_cast<uint64_t>(prod_lo_hi) + static_cast<uint64_t>(prod_hi_lo);
    const uint32_t W1 = static_cast<uint32_t>(W1_temp);
    const uint32_t carry_W1 = static_cast<uint32_t>(W1_temp >> 32);

    // W2 = prod_hi[63:32] + carry
    const uint32_t W2 = static_cast<uint32_t>(prod_hi >> 32) + carry_W1;

    // ---- 执行带舍入的右移：frac_part = ( (prod >> (q_num-1)) + 1 ) >> 1 ----
    const uint8_t t = q_num - 1;  // 中间移位数，0 ≤ t ≤ 62

    uint32_t shifted;
    if(t == 0) {
        // t=0: 不需要移位，直接取 W0
        shifted = W0;
    } else if(t < 32) {
        // 移位数在 (0, 32) 范围，结果取自 W0 和 W1
        shifted = (W0 >> t) | (W1 << (32 - t));
    } else if(t == 32) {
        // t=32: 结果完全取自 W1
        shifted = W1;
    } else {
        // 移位数在 (32, 62] 范围，结果取自 W1 和 W2
        shifted = (W1 >> (t - 32)) | (W2 << (64 - t));
    }

    // 舍入：(shifted + 1) >> 1，结果 < pow10_scale < 2^31，保证不超过32位
    uint32_t frac_part = (static_cast<uint32_t>(shifted) + 1) >> 1;

    // 检查舍入进位
    if(frac_part >= pow10_scale) {
        digit_part += 1;
        frac_part -= pow10_scale;
    }

    return {
        .digit_part = digit_part,
        .frac_part = frac_part
    };
}


// 0 <= q_num <= 64
[[nodiscard]] static constexpr char * _fmtnum_abs_fixedpoint64(
    char * __restrict p_str,
    uint64_t abs_value_bits,
    uint8_t precision,
    uint8_t q_num
) {
    // 安全限制precision，确保不超出表格范围
    constexpr size_t MAX_PRECISION = POW10_TABLE.size() - 1;
    if(precision > MAX_PRECISION) precision = MAX_PRECISION;

    const auto parts = [&] -> DigitFracPair64 {
        if(q_num == 0) [[unlikely]] {
            return {abs_value_bits, 0};
        }
        return depart_abs_fixedpoint64(abs_value_bits, precision, q_num);
    }();

    p_str = parts.fmt_str(p_str, precision);
    return p_str;
}

}