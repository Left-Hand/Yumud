#pragma once

#include "common.hpp"

namespace ymd::str{

static constexpr auto _dump_from_unsigned_fixed(
    uint32_t abs_value_bits, 
    const uint32_t precsion,
    const uint8_t Q
){

    struct Ret{
        uint32_t digit_part;
        uint32_t frac_part;
    };

    const uint32_t lower_mask = _calc_low_mask(Q);

    const uint32_t pow10_scale = POW10_TABLE[precsion];
    // 使用64位整数进行计算，避免溢出
    const uint64_t frac_scaled_bits = static_cast<uint64_t>(abs_value_bits & lower_mask) * pow10_scale;

    const uint32_t round_bit = _calc_low_mask(Q) & (~_calc_low_mask(Q-1));

    
    // 右移Q位提取小数部分（注意处理Q=0的情况）
    uint32_t frac_part;
    uint32_t digit_part;

    // 计算舍入（基于小数部分的精度）
    const bool need_upper_round = (static_cast<uint32_t>(frac_scaled_bits) & round_bit) != 0;

    frac_part = static_cast<uint32_t>(frac_scaled_bits >> Q) + need_upper_round;

    // 检查是否需要进位到整数部分
    const bool carry_to_int = (frac_part >= pow10_scale);
    if(Q != 32) [[likely]]{
        digit_part = (uint32_t(abs_value_bits) >> Q) + uint32_t(carry_to_int);
    }else{
        digit_part = uint32_t(carry_to_int);
    }

    // 如果发生进位，调整小数部分
    frac_part -= carry_to_int * pow10_scale;


    return Ret{
        .digit_part = digit_part, 
        .frac_part = frac_part
    };
}



static constexpr char * _fmtnum_unsigned_fixed_impl(
    char * p_str, 
    uint32_t abs_value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    // 安全限制precsion，确保不超出表格范围
    constexpr size_t MAX_PRECSION = POW10_TABLE.size() - 1;
    if(precsion > MAX_PRECSION) precsion = MAX_PRECSION;

    const auto res = _dump_from_unsigned_fixed(abs_value_bits, precsion, Q);

    const auto digit_part = res.digit_part;
    const auto frac_part = res.frac_part;
    p_str = _fmtnum_u32_r10(p_str, digit_part);

    if(precsion){
        p_str[0] = '.';
        p_str++;
        _fmtnum_u32_r10_padded(p_str, frac_part, precsion);
        p_str += precsion;
    }

    return p_str;
}

}