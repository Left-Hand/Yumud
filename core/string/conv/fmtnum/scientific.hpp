#pragma once

#include "common.hpp"
#include "unsigned_fixed.hpp"

namespace ymd::str{

struct ScientificParts{
    uint32_t frac_part;
    uint32_t digit_minor_number;
    int32_t exponent;
};

static constexpr ScientificParts _dump_from_unsigned_fixed_scientific(
    uint32_t abs_value_bits, 
    const uint8_t precsion,
    const uint8_t Q
){


    if(Q == 32) [[unlikely]] {
        return _dump_from_unsigned_fixed_scientific(abs_value_bits >> 1, precsion, 31);
    }

    const uint32_t int_part = static_cast<uint32_t>(abs_value_bits >> Q);
    int32_t exponent;

    uint32_t frac_part; 
    uint32_t digit_minor_number;
    if((int_part) == 0){// x < 1
        const uint32_t one_bits = 1u << Q;
        const uint32_t iter_times = _u32_num_digits_r10(one_bits / abs_value_bits);
        const auto scaler = POW10_TABLE[iter_times];
        exponent = -iter_times;

        {
            const auto res = _dump_from_unsigned_fixed(abs_value_bits * scaler, precsion, Q);
            frac_part = res.frac_part;
            digit_minor_number = res.digit_part;
        }
    }else if((int_part) >= 10){ // x >= 10

        uint32_t iter_times = _u32_num_digits_r10(int_part) - 1;
        const uint32_t scale_int = POW10_TABLE[iter_times];

        digit_minor_number = abs_value_bits / (scale_int << Q);
        const uint32_t frac_bits = (abs_value_bits - ((digit_minor_number * scale_int) << Q));
        const uint64_t frac_fixed = static_cast<uint64_t>(frac_bits) * POW10_TABLE[precsion - iter_times];
        const bool need_up_round = bool(static_cast<uint32_t>(frac_fixed) & (1u << (Q - 1)));
        //round
        exponent = iter_times;
        
        frac_part = (frac_fixed >> Q);
        frac_part += uint32_t(need_up_round);

    }else{
        // x >= 1 and x < 10
        {
            const auto res = _dump_from_unsigned_fixed(abs_value_bits, precsion, Q);
            frac_part = res.frac_part;
            digit_minor_number = res.digit_part;
        }
        exponent = 0;
    }

    if(digit_minor_number >= 10){
        digit_minor_number = 1;
        frac_part = str::div_10(frac_part);
        exponent++;
    }
    
    return {
        .frac_part = frac_part,
        .digit_minor_number = digit_minor_number,
        .exponent = exponent
    };
}



char * _fmtnum_unsigned_fixed_scientific_impl(
    char * p_str, 
    uint32_t abs_value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    // 安全限制precsion，确保不超出表格范围
    constexpr size_t MAX_PRECSION = std::size(POW10_TABLE) - 1;
    if(precsion > MAX_PRECSION) precsion = MAX_PRECSION;

    const auto res = _dump_from_unsigned_fixed_scientific(abs_value_bits, precsion, Q);

    const auto digit_minor_number = res.digit_minor_number;
    const auto frac_part = res.frac_part;
    const auto exponent = res.exponent;

    p_str[0] = static_cast<char>(digit_minor_number + '0');
    p_str[1] = '.';
    p_str+=2;

    _fmtnum_u32_r10_padded(p_str, frac_part, precsion);
    p_str += precsion;

    p_str[0] = 'e';
    p_str++;

    uint32_t unsigned_exponent;
    if(exponent < 0){
        p_str[0] = '-';
        p_str++;
        unsigned_exponent = -exponent;
    }else{
        unsigned_exponent = exponent;
    }

    p_str = _fmtnum_u32_r10(p_str, unsigned_exponent);
    return p_str;
}

}