#pragma once

#include <cstdint>

#include <concepts>
#include <tuple>

#include "core/math/iq/iq_t.hpp"

namespace ymd::strconv2 {

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
iq_t<Q> atoq(const char * str, const size_t len){
    auto [int_part, frac_part, scale] = strconv::disassemble_fstr(str, len);
	
    return iq_t<Q>(int_part) + iq_t<Q>(_iq<Q>::from_i32((frac_part << Q) / scale));
}



void strconv::reverse_str(char * str, size_t len){
	if(len == 0) return;

	len -= 1;
	for(size_t i = 0; i < len / 2 + (len % 2); i++){
		std::swap(str[i],str[len - i]);
	}

	str[len + 1] = '\0';
}

std::tuple<int, int, int> strconv::disassemble_fstr(const char * str, const size_t len){
	
    int int_part = 0;
	int frac_part = 0;
	int scale = 1;

	bool into_f = false;
    bool minus = false;

	size_t base = 0;

	for(;base < len; base++){
		char chr = str[base];

		switch(chr){
			case '\0':
				goto ret;
			case '.':
				into_f = true;
				break;
			case '-':
				minus = true;
				break;
			case '+':
			default:
				goto conv;
		}
	}

conv:
	for(;base < len; base++){
		char chr = str[base];

		switch(chr){
			case '\0':
				goto ret;
			case '.':
				into_f = true;
				break;
			case '-':
			case '+':
				break;
			default:
				if(is_digit(chr)){
					if(into_f){
						frac_part = frac_part * 10 + (chr - '0');
						scale *= 10;
					}else{
						int_part = int_part * 10 + (chr - '0');
					}
				}else{
					goto ret;
				}
				break;
		}
	}

	if(minus){
		int_part = -int_part;
		frac_part = -frac_part;
	}

ret:
	return {int_part, frac_part, scale};
}

bool strconv::is_numeric(const char* str, const size_t len) {
	bool hasDigit = false;
	bool hasDot = false;
	bool hasSign = false;

	for (size_t i = 0; i < len; i++) {
		char chr = str[i];
		if(chr == '\0'){
			break;
		} else if (is_digit(chr)) {
			hasDigit = true;
		} else if (chr == '.') {
			if (hasDot || !hasDigit) {
				return false; // ���С�������С����ǰû�����֣����� false
			}
			hasDot = true;
		} else if (chr == '+' || chr == '-') {
			if (hasSign || hasDigit || hasDot) {
				return false; // ��������Ż��������ų���������֮�󣬷��� false
			}
			hasSign = true;
		} else {
			return false; // �����֡������Ż�С���㣬���� false
		}
	}
	return hasDigit;
}

bool strconv::is_digit(const char * str, const size_t len){
    for(size_t i = 0; i < len; i++){
		char chr = str[i];
        if(!is_digit(chr)) return false;
		if(chr == '\0') break;
    }
    return true;
}

}