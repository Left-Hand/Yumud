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


[[nodiscard]] size_t _uqtoa(const uint32_t abs_value_bits, char * const str, uint8_t precsion, const uint8_t Q);

template<typename D>
requires(sizeof(D) <= 4)
[[nodiscard]] size_t _qtoa(const D value_bits, char * str, uint8_t precsion, const uint8_t Q){
	using unsigned_type = std::make_unsigned_t<D>;
	using bits_type = std::conditional_t<
		std::is_signed_v<D>, 
		std::make_signed_t<D>, 
		std::make_unsigned_t<D>
	>;

	unsigned_type abs_value_bits;
	size_t ind = 0;
	if constexpr(std::is_signed_v<D>){
		const bool is_negative = std::bit_cast<bits_type>(value_bits) < 0;
		if(is_negative){
			str[0] = '-';
			str++;
			// abs_value_bits = static_cast<unsigned_type>(-value_bits);
			abs_value_bits = static_cast<unsigned_type>(-(value_bits + 1)) + 1;
		}else{
			abs_value_bits = static_cast<unsigned_type>(value_bits);
		}
	}else{
		abs_value_bits = value_bits;
	}
    
    return ind + _uqtoa(abs_value_bits, str + ind, precsion, Q);
}

template<typename D>
requires(sizeof(D) <= 4)
[[nodiscard]] size_t qtoa(const D bits, const size_t Q, char * const str, uint8_t precsion){
	using size_aligned_t = std::conditional_t<std::is_signed_v<D>, int32_t, uint32_t>;
	static_assert(sizeof(size_aligned_t) == sizeof(D));
	if constexpr(std::is_signed_v<D>)
		return _qtoa(int32_t(bits), str, precsion, Q);
	else
		return _uqtoa(uint32_t(bits), str, precsion, Q);
}

}