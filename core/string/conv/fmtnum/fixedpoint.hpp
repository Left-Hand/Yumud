#pragma once

#include "decimal.hpp"

namespace ymd::str{


struct [[nodiscard]] alignas(size_t) DigitFracPair final{
    uint32_t digit_part;
    uint32_t frac_part;


    __attribute__((always_inline))
    [[nodiscard]] constexpr char * fmt_str(char * p_str, const uint8_t precision) const noexcept {
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

// 0 < q_num <= 32
__attribute__((optimize("Ofast")))
[[nodiscard]] static constexpr DigitFracPair depart_abs_fixedpoint32(
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

// 0 < q_num <= 64
__attribute__((optimize("Ofast")))
[[nodiscard]] static constexpr DigitFracPair depart_abs_fixedpoint64(
    uint64_t abs_value_bits, 
    uint8_t precision,
    uint8_t q_num
){
    uint32_t digit_part = (uint32_t(abs_value_bits) >> q_num);
    
    uint32_t frac_part;

    return {
        .digit_part = digit_part, 
        .frac_part = frac_part
    };
}


// 0 <= q_num <= 32
[[nodiscard]] static constexpr char * _fmtnum_abs_fixedpoint32(
    char * p_str, 
    uint32_t abs_value_bits, 
    uint8_t precision, 
    uint8_t q_num
){
    // 安全限制precision，确保不超出表格范围
    constexpr size_t MAX_PRECSION = POW10_TABLE.size() - 1;
    if(precision > MAX_PRECSION) precision = MAX_PRECSION;

    const auto parts = [&] -> DigitFracPair{
        if(q_num == 0)[[unlikely]]{
            return {abs_value_bits, 0};
        }
        return depart_abs_fixedpoint32(abs_value_bits, precision, q_num);
    }();

    p_str = parts.fmt_str(p_str, precision);
    return p_str;
}

}