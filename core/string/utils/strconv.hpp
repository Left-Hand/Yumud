#pragma once

#include <cstdint>

#include <concepts>
#include <tuple>

#include "core/math/iq/fixed_t.hpp"

namespace ymd::strconv {

void reverse_str(char * str,const size_t len);
::std::tuple<int,int,int> disassemble_fstr(const char * str, const size_t len);
int kmp_find(char *src, const size_t src_len, const char *match, const size_t match_len);
void str_replace(char *src, const size_t src_len, const char *match, const char *replace, const size_t dst_len);

bool is_numeric(const char * str, const size_t len);
bool is_digit(const char * str, const size_t len);
__fast_inline bool is_digit(const char chr){return chr >= '0' && chr <= '9';}
__fast_inline bool is_alpha(const char chr) {
    return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
}

size_t itoa(int32_t value, char * str, uint8_t radix);
size_t iutoa(uint64_t value, char * str, uint8_t radix);
size_t iltoa(int64_t value, char * str, uint8_t radix);


size_t ftoa(float value, char * str, uint8_t eps);



int atoi(const char * str, const size_t len);
float atof(const char * str, const size_t len);


__inline int atoi(const char * str){return atoi(str, strlen(str));}
__inline float atof(const char * str){return atof(str, strlen(str));};

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

template<size_t Q>
fixed_t<Q, int32_t> atoq(const char * str, const size_t len){
    auto [int_part, frac_part, scale] = strconv::disassemble_fstr(str, len);
	
    return fixed_t<Q, int32_t>(int_part) + fixed_t<Q, int32_t>::from_bits((frac_part << Q) / scale);
}

size_t _qtoa_impl(const int32_t value_, char * str, uint8_t eps, const uint8_t _Q);

template<size_t Q>
size_t qtoa(const fixed_t<Q, int32_t> & qv, char * str, uint8_t eps){
    return _qtoa_impl(qv.to_bits(), str, eps, Q);
}

}