#pragma once

#include "core/platform.hpp"
#include <cstdint>

#include <concepts>
#include <tuple>

#include "core/math/iq/fixed_t.hpp"

namespace ymd::str{
[[nodiscard]] __fast_inline constexpr 
bool is_digit(const char chr){return chr >= '0' && chr <= '9';}
[[nodiscard]] __fast_inline constexpr 
bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

[[nodiscard]] char * fmtstr_i32(char * str, const int32_t value, uint8_t radix);
[[nodiscard]] char * fmtstr_u32(char * str, const uint32_t value, uint8_t radix);
[[nodiscard]] char * fmtstr_u64(char * str, const uint64_t value, uint8_t radix);
[[nodiscard]] char * fmtstr_i64(char * str, const int64_t value, uint8_t radix);
[[nodiscard]] char * fmtstr_f32(char * str, const float value, uint8_t precsion);


[[nodiscard]] char * _fmtstr_unsigned_fixed_impl(char * str, const uint32_t abs_value_bits, uint8_t precsion, const uint8_t Q);

[[nodiscard]] char * _fmtstr_signed_fixed_impl(char * str, const int32_t value_bits, uint8_t precsion, const uint8_t Q);

template<typename D>
requires(sizeof(D) <= 4)
[[nodiscard]] char * fmtstr_fixed(
    char * const str, 
    const D bits, 
    uint8_t precsion, 
    const size_t Q
){
	using size_aligned_t = std::conditional_t<std::is_signed_v<D>, int32_t, uint32_t>;
	static_assert(sizeof(size_aligned_t) == sizeof(D));
	if constexpr(std::is_signed_v<D>)
		return _fmtstr_signed_fixed_impl(str, int32_t(bits), precsion, Q);
	else
		return _fmtstr_unsigned_fixed_impl(str, uint32_t(bits), precsion, Q);
}


__attribute__((always_inline))

[[nodiscard]] static constexpr uint32_t  _div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = (1ull << SHIFTS) / 100000 + 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}

static_assert(_div_100000(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 100000);

__attribute__((hot))
static constexpr size_t _u32_num_digits_r10(uint32_t int_val){
    if(int_val == 0) [[unlikely]] return 1;

    auto match_result = [&](const uint32_t int_val_scaled) -> size_t __attribute__((always_inline)){
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

}