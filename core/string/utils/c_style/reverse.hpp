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
	if(len == 0) return;

	len -= 1;
    const size_t mid = len / 2 + (len % 2);
	for(size_t i = 0; i < mid; i++){
		swap_char(str[i],str[len - i]);
	}
}
}