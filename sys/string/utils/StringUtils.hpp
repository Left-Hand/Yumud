#pragma once

#include "stdint.h"
#include "string.h"

#include <concepts>
#include <tuple>

#include "sys/math/iq/iq_t.hpp"

namespace ymd::StringUtils {


scexpr uint32_t scale_map[] = {1UL, 10UL, 100UL, 1000UL, 10000UL, 100000UL, 1000000UL, 10000000UL};


template<typename T>
__fast_inline constexpr size_t _get_scalar(T value, const size_t radix){
    if(value == 0) return 1;

    if(radix == 10){
        size_t scalar = 0;
        value = ABS(value);
        
        while(value > 1000000){
            value /= 1000000;
            scalar += 6;
        }
        
        size_t i = 0;
        while(uint32_t(value) >= scale_map[i]) i++;
        return scalar + i;
    }else{
        size_t i = 0;
        size_t sum = 1;
        while(size_t(value) > sum){
            sum *= radix;
            i++;
        }
        return MAX(i, 1);
    }
}



template<integral T>
size_t _itoa_impl(T value, char * str, uint8_t radix){
    const bool minus = value < 0;

    value = ABS(value);

    const size_t len = _get_scalar(value, radix) + minus;
    str[len] = 0;
    int i = len - 1;

    do {
		const uint8_t digit = value % radix;
        str[i] = ((digit) > 9) ? 
		(digit - 10) + ('A') : (digit) + '0';
        i--;
    } while((value /= radix) > 0 and (i >= 0));

    if(minus) {
        str[0] = '-';
    }

    return len;
}

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


template<size_t _Q>
size_t qtoa(const iq_t<_Q> value, char * str, uint8_t eps){
    //TODO 支持除了Q16格式外其他格式转换到字符串的函数 

    scexpr size_t Q = 16;
    eps = MIN(eps, 5);

	const bool minus = value < 0;
    const uint32_t abs_value = ABS((iq_t<Q>(value).value.to_i32()));
    const uint32_t lower_mask = (Q == 31) ? 0x7fffffffu : uint32_t(((1 << Q) - 1));

    const uint32_t frac_part = uint32_t(abs_value) & lower_mask;

    const uint32_t scale = scale_map[eps];

    const uint32_t fs = frac_part * scale;
    
    const bool upper_round = (fs & lower_mask) >= (lower_mask >> 1);

    const uint32_t frac_int = (fs >> Q) + upper_round;
    const uint32_t int_part = (uint32_t(abs_value) >> Q) + bool(frac_int >= scale);

    if(minus){
        str[0] = '-';
    }

    const auto end = _itoa_impl<int>(int_part, str + minus, 10) + minus;

    if(eps){
        str[end] = '.';
        //add dot to seprate
        itoas(frac_int, str + end + 1, 10, eps);
    }

    return end + 1 + eps;
}


template<size_t Q>
iq_t<Q> atoq(const char * str, const size_t len){
    auto [int_part, frac_part, scale] = StringUtils::disassemble_fstr(str, len);
	
    return iq_t<Q>(int_part) + iq_t<Q>(_iq<Q>::from_i32((frac_part << Q) / scale));
}

}