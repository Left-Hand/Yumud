#pragma once

#include <cstddef>

namespace ymd::str{
__attribute__((always_inline))
constexpr void swap_char(char & a, char & b){
    char tmp = a;
    a = b;
    b = tmp;
}

constexpr void reverse(char * str, size_t len){
	if(len == 0) __builtin_unreachable();

	len -= 1;
    const size_t mid = (len >> 1) + (len & 1);

	#pragma GCC unroll 8
	for(size_t i = 0; i < mid; i++){
		swap_char(str[i],str[len - i]);
	}
}
}