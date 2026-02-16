#pragma once

#include "core/math/iq/fixed_t.hpp"

namespace ymd::str{

[[nodiscard]] char * fmtnum_i32(char * p_str, const int32_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_u32(char * p_str, const uint32_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_u64(char * p_str, const uint64_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_i64(char * p_str, const int64_t value, uint8_t radix);
[[nodiscard]] char * fmtnum_f32(char * p_str, const float value, uint8_t precsion);

[[nodiscard]] char * _fmtnum_unsigned_fixed_impl(char * p_str, 
    const uint32_t abs_value_bits, uint8_t precsion, const uint8_t Q);

[[nodiscard]] char * _fmtnum_signed_fixed_impl(char * p_str, 
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
		return _fmtnum_signed_fixed_impl(str, int32_t(bits), precsion, Q);
	else
		return _fmtnum_unsigned_fixed_impl(str, uint32_t(bits), precsion, Q);
}


__attribute__((always_inline))

[[nodiscard]] static constexpr uint32_t  _div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = (1ull << SHIFTS) / 100000 + 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}

static_assert(_div_100000(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 100000);

__attribute__((always_inline, hot))
static constexpr size_t _u32_num_digits_r10(uint32_t int_val){
    if(int_val == 0) [[unlikely]] return 1;

    auto match_result = [&](const uint32_t int_val_scaled) 
        -> size_t __attribute__((always_inline)
    ){
        if(int_val_scaled >= 100) [[likely]] {
            if(int_val_scaled >= 10000) return 5;
            else if(int_val_scaled >= 1000) return 4;
            else return 3;
        }else{
            if(int_val_scaled >= 10) return 2;
            else return 1;
        }
    };

    if(int_val >= 100000){
        int_val = _div_100000(int_val);
        return 5u + match_result(int_val);
    }else{
        return match_result(int_val);
    }

}

//n <= 34
static constexpr uint32_t _div_3(const uint32_t n){
    constexpr size_t SHIFTS = 32;
    constexpr uint32_t MAGIC = ((1ull << SHIFTS) / 3 + 1);
    return uint32_t((uint64_t(n) * MAGIC) >> SHIFTS);
}

__attribute__((always_inline))
static constexpr char * put_basealpha(char * p_str, const uint32_t radix){
    switch(radix){
        default:
        case 10:
            return p_str;
        case 2:
            p_str[0] = '0';
            p_str[1] = 'b';
            return p_str + 2;
        case 8:
            p_str[0] = '0';
            return p_str + 1;
        case 16:
            p_str[0] = '0';
            p_str[1] = 'x';
            return p_str + 2;
    }
}

}