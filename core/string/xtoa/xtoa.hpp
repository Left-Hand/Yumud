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

[[nodiscard]] size_t itoa(int32_t value, char * str, uint8_t radix);
[[nodiscard]] size_t iutoa(uint64_t value, char * str, uint8_t radix);
[[nodiscard]] size_t iltoa(int64_t value, char * str, uint8_t radix);
[[nodiscard]] size_t ftoa(float value, char * str, uint8_t precsion);


[[nodiscard]] char * _uqtoa_impl(const uint32_t abs_value_bits, char * str, uint8_t precsion, const uint8_t Q);

[[nodiscard]] char * _iqtoa_impl(const int32_t value_bits, char * str, uint8_t precsion, const uint8_t Q);

template<typename D>
requires(sizeof(D) <= 4)
[[nodiscard]] size_t qtoa(const D bits, const size_t Q, char * const str, uint8_t precsion){
	using size_aligned_t = std::conditional_t<std::is_signed_v<D>, int32_t, uint32_t>;
	static_assert(sizeof(size_aligned_t) == sizeof(D));
	if constexpr(std::is_signed_v<D>)
		return _iqtoa_impl(int32_t(bits), str, precsion, Q) - str;
	else
		return _uqtoa_impl(uint32_t(bits), str, precsion, Q) - str;
}


__attribute__((always_inline))

[[nodiscard]] static constexpr uint32_t  _div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = (1ull << SHIFTS) / 100000 + 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}

static_assert(_div_100000(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 100000);


static constexpr size_t _u32_num_digits_r10(uint32_t int_val){
    if(int_val == 0) [[unlikely]] return 1;
    size_t len = 0;
    if(int_val >= 100000){
        int_val = _div_100000(int_val);
        len += 5;
    }


    if(int_val >= 100){
        if(int_val >= 10000) len += 5;
        else if(int_val >= 1000) len += 4;
        else len += 3;
    }else{
        if(int_val >= 10) len += 2;
        else len += 1;
    }

    return len;
}

//n <= 34
static constexpr uint32_t _div_3(const uint32_t n){
    constexpr size_t SHIFTS = 32;
    constexpr uint32_t MAGIC = ((1ull << SHIFTS) / 3 + 1);
    return uint32_t((uint64_t(n) * MAGIC) >> SHIFTS);
}

}