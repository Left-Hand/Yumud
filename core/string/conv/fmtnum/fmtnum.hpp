#pragma once

#include "core/math/fixed/fixed.hpp"
#include "decimal.hpp"
#include "nondecimal.hpp"
#include "fixedpoint.hpp"
#include "floatingpoint.hpp"

namespace ymd::str{



constexpr char * _fmtnum_mayneg_i32_r10(char * p_str, uint32_t int_val, const IntTypeErased type){
    if (type.is_signed) {
        const bool is_negative = std::bit_cast<int32_t>(int_val) < 0;
        if(is_negative) {
            p_str[0] = '-';
            p_str++;
            // 安全地取绝对值
            int_val = static_cast<uint32_t>(-std::bit_cast<int32_t>(int_val));
        }
    } 

    return _fmtnum_u32_r10_fittest(p_str, int_val);
}

// template<typename T>
// [[nodiscard]] constexpr char * fmtnum32_template(char * p_str, const T int_val, uint8_t radix){
//     static constexpr IntTypeErased type = IntTypeErased::from<T>();
//     switch(radix){
//         case 10:
//             return _fmtnum_mayneg_i32_r10(p_str, static_cast<uint32_t>(int_val), type);
//         case 16:{
//             const size_t len = type.width / 4;
//             _fmtnum_u32_r16(p_str, static_cast<uint32_t>(int_val), len);
//             return p_str + len;
//         }
//         case 8:
//             // return _fmtnum_u32_r8(p_str, static_cast<uint32_t>(int_val));
//             //TODO
//             return p_str;
//         case 2:{
//             const size_t len = type.width;
//             _fmtnum_u32_r2(p_str, static_cast<uint32_t>(int_val), len);
//             return p_str + len;
//         }
//     }
    
//     //no chars 
//     return p_str;
// }

[[nodiscard]] char * fmtnum32_erased(
    char * p_str, 
    const uint32_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
);

[[nodiscard]] char * fmtnum64_erased(
    char * p_str, 
    const uint64_t int_val, 
    uint8_t radix, 
    const IntTypeErased type
);

[[nodiscard]] char * fmtnum_f32(char * p_str, const float value, uint8_t precsion);

[[nodiscard]] char * _fmtnum_signed_fixed(char * p_str, 
    const uint32_t value_bits, uint8_t precsion, const uint8_t Q);

[[nodiscard]] char * _fmtnum_unsigned_fixed(char * p_str, 
    const uint32_t value_bits, uint8_t precsion, const uint8_t Q);

template<typename D>
requires(sizeof(D) <= 4)
[[nodiscard]] char * fmtnum_fixed(
    char * const str, 
    const D bits, 
    uint8_t precsion, 
    const size_t Q
){
	using size_aligned_t = std::conditional_t<std::is_signed_v<D>, int32_t, uint32_t>;
	static_assert(sizeof(size_aligned_t) == sizeof(D));
	if constexpr(std::is_signed_v<D>)
		return _fmtnum_signed_fixed(str, std::bit_cast<uint32_t>(static_cast<int32_t>(bits)), precsion, Q);
	else
		return _fmtnum_unsigned_fixed(str, static_cast<uint32_t>(bits), precsion, Q);
}


}