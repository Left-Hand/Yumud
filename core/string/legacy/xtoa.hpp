#pragma once

#include "core/platform.hpp"
#include <cstdint>

#include <concepts>
#include <tuple>

#include "core/math/iq/fixed_t.hpp"

namespace ymd::str{
__fast_inline bool is_digit(const char chr){return chr >= '0' && chr <= '9';}
__fast_inline bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

size_t itoa(int32_t value, char * str, uint8_t radix);
size_t iutoa(uint64_t value, char * str, uint8_t radix);
size_t iltoa(int64_t value, char * str, uint8_t radix);
size_t ftoa(float value, char * str, uint8_t precsion);


static __fast_inline constexpr void utoas(uint32_t value, char * const str, uint8_t radix, int8_t i)  {
    i -= 1;
	do{
		const uint8_t digit = value % radix;
		str[i] = ((digit) > 9) ? 
		(digit - 10) + ('A') : (digit) + '0';

		i--;
        value /= radix;
	}while(i >= 0);
}

size_t _uqtoa(const uint32_t abs_value_bits, char * const str, uint8_t precsion, const uint8_t Q);

template<typename D>	
size_t _iqtoa(const D value_bits, char * str, uint8_t precsion, const uint8_t Q){
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
			ind = 1;
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

template<size_t Q, typename D>
requires(sizeof(D) <= 4)
size_t qtoa(const math::fixed_t<Q, D> qv, char * const str, uint8_t precsion){
	using size_aligned_t = std::conditional_t<std::is_signed_v<D>, int32_t, uint32_t>;
	static_assert(sizeof(size_aligned_t) == sizeof(D));
	return _iqtoa<size_aligned_t>(std::bit_cast<size_aligned_t>(qv.to_bits()), str, precsion, Q);
}

}