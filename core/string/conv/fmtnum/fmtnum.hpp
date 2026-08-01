#pragma once

#include "decimal.hpp"
#include "nondecimal.hpp"
#include "fixedpoint.hpp"
#include "floatingpoint.hpp"

namespace ymd::str{



[[nodiscard]] constexpr char * _fmtnum_mayneg_i32_dec(
    char * __restrict p_str, 
    int32_t int_val, 
    const IntTypeErased type
){
    if (type.is_signed) {
        const bool is_negative = int_val < 0;
        if(is_negative) {
            p_str[0] = '-';
            p_str++;
            // 安全地取绝对值
            int_val = -std::bit_cast<int32_t>(int_val);
        }
    } 

    return _fmtnum_u32_dec_fittest(p_str, uint32_t(int_val));
}

[[nodiscard]] constexpr char * _fmtnum_mayneg_i64_dec(
    char * __restrict p_str, 
    int64_t int_val, 
    const IntTypeErased type
){
    if (type.is_signed) {
        const bool is_negative = int_val < 0;
        if(is_negative) {
            p_str[0] = '-';
            p_str++;
            // 安全地取绝对值
            int_val = -std::bit_cast<int64_t>(int_val);
        }
    } 

    return _fmtnum_u64_dec_fittest(p_str, uint64_t(int_val));
}

[[nodiscard]] constexpr char * fmtnum_integral32(
    char * __restrict p_str, 
    const uint32_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
){
    switch(radix){
        case 10:{
            char * end = _fmtnum_mayneg_i32_dec(p_str, static_cast<uint32_t>(int_val), type);
            return end;
        }
        case 16:{
            const size_t len = type.width / 4;
            char * end = p_str + len;
            _fmtnum_u32_hex({p_str, end}, static_cast<uint32_t>(int_val));
            return end;
        }
        case 8:{
            const size_t len = _least_u32_num_digits_oct(static_cast<uint32_t>(int_val));
            char * end = p_str + len;
            _fmtnum_u32_oct({p_str, end}, static_cast<uint32_t>(int_val));
            return end;
        }
        case 2:{
            const size_t len = type.width;
            char * end = p_str + len;
            _fmtnum_u32_bin({p_str, end}, static_cast<uint32_t>(int_val));
            return end;
        }
    }
    
    //no chars 
    return p_str;
}


[[nodiscard]] 
__attribute__((__noinline__))
constexpr char * fmtnum_integral64(
    char * __restrict p_str, 
    const uint64_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
){
    switch(radix){
        case 10:{
            char * end = _fmtnum_mayneg_i64_dec(p_str, static_cast<int64_t>(int_val), type);
            return end;
        }
        case 16:{
            constexpr size_t NUM_DIGITS_U32 = (sizeof(uint32_t) * 8) / 4;
            const uint32_t high32 = static_cast<uint32_t>(int_val >> 32);
            const uint32_t low32 = static_cast<uint32_t>(int_val);

            _fmtnum_u32_hex({p_str, p_str + NUM_DIGITS_U32}, high32);
            _fmtnum_u32_hex({p_str + NUM_DIGITS_U32, p_str + 2 * NUM_DIGITS_U32}, low32);
            char * end = p_str + (NUM_DIGITS_U32 * 2);
            return end;
        }
        case 8:{
            // return _fmtnum_u32_oct(p_str, static_cast<uint32_t>(int_val));
            //TODO
            return p_str;
        }
        case 2:{
            constexpr size_t NUM_DIGITS_U32 = (sizeof(uint32_t) * 8) / 1;
            const uint32_t high32 = static_cast<uint32_t>(int_val >> 32);
            const uint32_t low32 = static_cast<uint32_t>(int_val);

            _fmtnum_u32_hex({p_str, p_str + NUM_DIGITS_U32}, high32);
            _fmtnum_u32_hex({p_str + NUM_DIGITS_U32, p_str + 2 * NUM_DIGITS_U32}, low32);
            char * end = p_str + (NUM_DIGITS_U32 * 2);
            return end;
        }
    }

    //no chars 
    return p_str;
}


[[nodiscard]] constexpr char * fmtnum_f32(char * __restrict p_str, const float value, uint8_t precision){
    return _fmtnum_f32(p_str, value, precision);
}



[[nodiscard]] constexpr char * fmtnum_fixedpoint32(
    char * __restrict p_str, 
    int32_t bits, 
    uint8_t precision, 
    FixedTypeTag type
){
    if(type.is_signed){
        const bool is_negative = bits < 0;

        if(is_negative){
            p_str[0] = '-';
            p_str++;
            bits = -bits;
        }
    }

    return _fmtnum_abs_fixedpoint32(p_str, static_cast<uint32_t>(bits), precision, type.q_num);
}

[[nodiscard]] constexpr char * fmtnum_fixedpoint64(
    char * __restrict p_str, 
    int64_t bits, 
    uint8_t precision, 
    FixedTypeTag type
){
    if(type.is_signed){
        const bool is_negative = bits < 0;

        if(is_negative){
            p_str[0] = '-';
            p_str++;
            bits = -bits;
        }
    }

    return _fmtnum_abs_fixedpoint64(p_str, static_cast<uint64_t>(bits), precision, type.q_num);
}


}