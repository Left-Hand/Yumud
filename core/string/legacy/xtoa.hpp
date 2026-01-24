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


static __fast_inline constexpr void itoas(uint32_t value, char *str, uint8_t radix, int8_t i)  {
    i -= 1;
	do{
		const uint8_t digit = value % radix;
		str[i] = ((digit) > 9) ? 
		(digit - 10) + ('A') : (digit) + '0';

		i--;
        value /= radix;
	}while(i >= 0);
}

size_t _qtoa_impl(const int32_t value_, char * str, uint8_t eps, const uint8_t _Q);

template<size_t Q>
size_t qtoa(const math::fixed_t<Q, int32_t> qv, char * str, uint8_t eps){
    return _qtoa_impl(qv.to_bits(), str, eps, Q);
}

}