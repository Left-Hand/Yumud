#include "fmtnum.hpp"
#include "common.hpp"
#include "non10_radix.hpp"
#include "unsigned_fixed.hpp"
#include "floating.hpp"

#include <array>

using namespace ymd;
using namespace ymd::str;




template<bool IS_SIGNED>
constexpr char * _fmtnum_int32_impl(char * p_str, uint32_t int_val, uint8_t radix){
    auto preprocess_ifneg = [&]{
        if constexpr (IS_SIGNED) {
            const bool is_negative = int32_t(int_val) < 0;
            if(is_negative) {
                p_str[0] = '-';
                p_str++;
                // 安全地取绝对值
                int_val = static_cast<uint32_t>(-int32_t(int_val));
            }
        } 
    };

    switch(radix){
        case 10:
            preprocess_ifneg();
            return _fmtnum_u32_r10(p_str, int_val);
        case 16:
            return _fmtnum_u32_r16(p_str, int_val);
        case 8:
            return _fmtnum_u32_r8(p_str, int_val);
        case 2:
            return _fmtnum_u32_r2(p_str, int_val);
        default:
            //no chars 
            return p_str;
    }

    __builtin_unreachable();
}

template<typename T>
requires (sizeof(T) <= 4)
constexpr char * _fmtnum_int32(char * p_str, T int_val, uint8_t radix){
    return _fmtnum_int32_impl<std::is_signed_v<T>>(p_str, static_cast<uint32_t>(int_val), radix);
}

template<typename T>
requires ((sizeof(T) == 8))
constexpr char * _fmtnum_int64(char * p_str, T int_val, uint8_t radix){
    //TODO support 64bit
    return _fmtnum_int32_impl<std::is_signed_v<T>>(p_str, static_cast<uint32_t>(int_val), radix);
}

char * str::fmtnum_i32(
    char *p_str, 
    int32_t int_val,
    uint8_t radix
){
    return _fmtnum_int32<int32_t>(p_str, int_val, radix);
}

char * str::fmtnum_u32(
    char *p_str, 
    uint32_t int_val,
    uint8_t radix
){
    return _fmtnum_int32<uint32_t>(p_str, int_val, radix);
}


char * str::fmtnum_u64(
    char *p_str,
    uint64_t int_val,
    uint8_t radix
){
    if(bool(int_val >> 32) == false){
        return _fmtnum_int32<uint32_t>(p_str, int_val, radix);
    }

    //TODO 64位除法的实现会大幅增大体积
    return _fmtnum_int64<uint64_t>(p_str, int_val, radix);
}


char * str::fmtnum_i64(
    char * p_str, 
    int64_t int_val, 
    uint8_t radix
){
    const uint32_t high32 = static_cast<uint32_t>(int_val >> 32);

    //u32
    if(high32 == 0) [[likely]]{
        return _fmtnum_int32<uint32_t>(p_str, static_cast<uint32_t>(int_val), radix);
    }

    //neg i32
    if(high32 == UINT32_MAX) [[likely]]{
        p_str[0] = '-';
        p_str++;
        return _fmtnum_int32<uint32_t>(p_str, static_cast<uint32_t>(-int_val), radix);
    }

    return _fmtnum_int64<int64_t>(p_str, int_val, radix);
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

    return _fmtnum_unsigned_fixed_impl(p_str, value_bits, precsion, Q);
}


char * str::_fmtnum_unsigned_fixed(
    char * p_str, 
    const uint32_t value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    return _fmtnum_unsigned_fixed_impl(p_str, value_bits, precsion, Q);
}
