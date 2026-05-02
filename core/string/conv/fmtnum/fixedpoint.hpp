#pragma once

#include "decimal.hpp"

namespace ymd::str{


struct [[nodiscard]] DigitFracPair final{
    uint32_t digit_part;
    uint32_t frac_part;


    __attribute__((always_inline))
    [[nodiscard]] constexpr char * fmt_str(char * p_str, const uint8_t precision) const {
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

// 0 < Q <= 32
__attribute__((optimize("Ofast")))
[[nodiscard]] static constexpr DigitFracPair depart_abs_fixedpoint(
    uint32_t abs_value_bits, 
    uint8_t precision,
    uint8_t Q
){

    if(Q > 32) __builtin_unreachable();

    if(Q == 32) [[unlikely]]{
        Q = 31;
        abs_value_bits >>= 1;
    }

    const uint32_t lower_mask = (1u << Q) - 1;

    const uint32_t pow10_scale = POW10_TABLE[precision];
    // 使用64位整数进行计算，避免溢出
    const uint64_t frac_scaled_bits = static_cast<uint64_t>(abs_value_bits & lower_mask) * pow10_scale;

    // 右移Q位提取小数部分（注意处理Q=0的情况）
    uint32_t frac_part = (static_cast<uint32_t>(frac_scaled_bits >> (Q - 1)) + 1) >> 1;
    uint32_t digit_part = (uint32_t(abs_value_bits) >> Q);


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


// 0 < Q <= 32
[[nodiscard]] static constexpr char * _fmtnum_abs_fixedpoint(
    char * p_str, 
    uint32_t abs_value_bits, 
    uint8_t precision, 
    uint8_t Q
){
    // 安全限制precision，确保不超出表格范围
    constexpr size_t MAX_PRECSION = POW10_TABLE.size() - 1;
    if(precision > MAX_PRECSION) precision = MAX_PRECSION;


    const auto parts = depart_abs_fixedpoint(abs_value_bits, precision, Q);

    p_str = parts.fmt_str(p_str, precision);
    return p_str;
}

}