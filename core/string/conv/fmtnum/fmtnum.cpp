#include "fmtnum.hpp"


#include <array>

using namespace ymd;
using namespace ymd::str;



[[nodiscard]] char * str::fmtnum32_erased(
    char * p_str, 
    const uint32_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
){
    switch(radix){
        case 10:
            return _fmtnum_mayneg_i32_r10(p_str, static_cast<uint32_t>(int_val), type);
        case 16:{
            const size_t len = type.width / 4;
            _fmtnum_u32_r16(p_str, static_cast<uint32_t>(int_val), len);
            return p_str + len;
        }
        case 8:
            // return _fmtnum_u32_r8(p_str, static_cast<uint32_t>(int_val));
            //TODO
            return p_str;
        case 2:{
            const size_t len = type.width;
            _fmtnum_u32_r2(p_str, static_cast<uint32_t>(int_val), len);
            return p_str + len;
        }
    }
    
    //no chars 
    return p_str;
}


[[nodiscard]] char * str::fmtnum64_erased(
    char * p_str, 
    const uint64_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
){


    const uint32_t high32 = static_cast<uint32_t>(int_val >> 32);
    const uint32_t low32 = static_cast<uint32_t>(int_val);


    switch(radix){
        case 10:
            //u32
            if(high32 == 0) [[likely]]{
                return fmtnum32_erased(p_str, static_cast<uint32_t>(int_val),  
                    radix, IntTypeErased::from<uint32_t>());
            }

            if(type.is_signed){
                //neg i32
                if(high32 == UINT32_MAX) [[likely]]{
                    p_str[0] = '-';
                    p_str++;
                    return fmtnum32_erased(p_str, static_cast<uint32_t>(-int_val), 
                        radix, IntTypeErased::from<uint32_t>());
                }
            }
            
            //TODO support 64bit
            return _fmtnum_mayneg_i32_r10(p_str, static_cast<uint32_t>(int_val), type);
        case 16:{
            constexpr size_t NUM_DIGITS_U32 = (sizeof(uint32_t) * 8) / 4;
            _fmtnum_u32_r16(p_str, high32, NUM_DIGITS_U32);
            _fmtnum_u32_r16(p_str, low32, NUM_DIGITS_U32);
            return p_str + (NUM_DIGITS_U32 * 2);
        }
        case 8:
            // return _fmtnum_u32_r8(p_str, static_cast<uint32_t>(int_val));
            //TODO
            return p_str;
        case 2:{
            constexpr size_t NUM_DIGITS_U32 = (sizeof(uint32_t) * 8) / 1;
            _fmtnum_u32_r2(p_str, high32, NUM_DIGITS_U32);
            _fmtnum_u32_r2(p_str, low32, NUM_DIGITS_U32);
            return p_str + (NUM_DIGITS_U32 * 2);
        }
        default:
            //no chars 
            return p_str;
    }
}


char * str::fmtnum_f32(
    char * p_str, 
    float float_val, 
    uint8_t precision
){
    return _fmtnum_f32_impl(p_str, float_val, precision);
}

char * str::_fmtnum_signed_fixed(
    char * p_str, 
    uint32_t value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    const bool is_negative = std::bit_cast<int32_t>(value_bits) < 0;

    if(is_negative){
        p_str[0] = '-';
        p_str++;
        value_bits = static_cast<uint32_t>(-std::bit_cast<int32_t>(value_bits));
    }

    return _fmtnum_abs_fixedpoint(p_str, value_bits, precsion, Q);
}


char * str::_fmtnum_unsigned_fixed(
    char * p_str, 
    const uint32_t value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    return _fmtnum_abs_fixedpoint(p_str, value_bits, precsion, Q);
}
