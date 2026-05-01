#pragma once

#include "core/math/fixed/fixed.hpp"
#include "decimal.hpp"
#include "nondecimal.hpp"
#include "fixedpoint.hpp"
#include "floatingpoint.hpp"

namespace ymd::str{



[[nodiscard]] constexpr char * _fmtnum_mayneg_i32_dec(char * p_str, uint32_t int_val, const IntTypeErased type){
    if (type.is_signed) {
        const bool is_negative = std::bit_cast<int32_t>(int_val) < 0;
        if(is_negative) {
            p_str[0] = '-';
            p_str++;
            // 安全地取绝对值
            int_val = static_cast<uint32_t>(-std::bit_cast<int32_t>(int_val));
        }
    } 

    return _fmtnum_u32_dec_fittest(p_str, int_val);
}

[[nodiscard]] constexpr char * fmtnum_integral32(
    char * p_str, 
    const uint32_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
){
    switch(radix){
        case 10:
            return _fmtnum_mayneg_i32_dec(p_str, static_cast<uint32_t>(int_val), type);
        case 16:{
            const size_t len = type.width / 4;
            char * end = p_str + len;
            _fmtnum_u32_r16({p_str, end}, static_cast<uint32_t>(int_val));
            return end;
        }
        case 8:{
            const size_t len = _least_u32_num_digits_r8(static_cast<uint32_t>(int_val));
            char * end = p_str + len;
            _fmtnum_u32_r8({p_str, end}, static_cast<uint32_t>(int_val));
            return end;
        }
        case 2:{
            const size_t len = type.width;
            char * end = p_str + len;
            _fmtnum_u32_r2({p_str, end}, static_cast<uint32_t>(int_val));
            return end;
        }
    }
    
    //no chars 
    return p_str;
}


[[nodiscard]] char * fmtnum_integral64(
    char * p_str, 
    const uint64_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
);

[[nodiscard]] char * fmtnum_f32(char * p_str, const float value, uint8_t precsion);


struct [[nodiscard]] alignas(4) FixedTypeErased final{
    bool is_signed;
    const uint8_t q_num;
};

[[nodiscard]] constexpr char * fmtnum_fixedpoint(
    char * p_str, 
    uint32_t bits, 
    uint8_t precsion, 
    FixedTypeErased type
){
    if(type.is_signed){
        const bool is_negative = std::bit_cast<int32_t>(bits) < 0;

        if(is_negative){
            p_str[0] = '-';
            p_str++;
            bits = static_cast<uint32_t>(-std::bit_cast<int32_t>(bits));
        }
    }

    return _fmtnum_abs_fixedpoint(p_str, static_cast<uint32_t>(bits), precsion, type.q_num);
}


}