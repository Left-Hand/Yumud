#include "StringUtils.hpp"
#include <array>

using namespace ymd;
using namespace ymd::StringUtils;

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


size_t ymd::StringUtils::_qtoa_impl(int32_t value, char * str, uint8_t eps, const uint8_t _Q){
    //TODO 支持除了Q16格式外其他格式转换到字符串的函数 
    scexpr size_t Q = 16;


    value = RSHIFT(value, _Q - Q);
    eps = MIN(eps, 5);

	const bool minus = value < 0;
    const uint32_t abs_value = ABS(value);
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

size_t StringUtils::itoa(int32_t value, char *str, uint8_t radix){
    return _itoa_impl<int32_t>(value, str, radix);
}


size_t StringUtils::iutoa(uint64_t value,char *str,uint8_t radix){
    // if(value > INT32_MAX or value < INT32_MIN){
    //     return _itoa_impl<int32_t>(value, str, radix);
    // }
    return _itoa_impl(value, str, radix);
}


size_t iltoa(int64_t value, char * str, uint8_t radix){
    return _itoa_impl<int64_t>(value, str, radix);
}

size_t StringUtils::ftoa(float number,char *buf, uint8_t eps)
{
    char str_int[12] = {0};
    char str_float[eps+1] = {0};

    long int_part = (long)number;
    float float_part = number - (float)int_part;

	if(number < 0 && int_part == 0){
		str_int[0] = '-';
		itoa(int_part,str_int + 1,10);
	}
	else itoa(int_part,str_int,10);

    if(eps){
        float scale = 1;
        for(uint8_t i = 0; i < eps; i++)
            scale *= 10;

        float_part *= scale;
        itoas((int)(float_part),str_float, 10, eps);
    }

    int i = strlen(str_int);
    str_int[i] = '.';
    strcat(str_int,str_float);
    strcpy(buf,str_int);

    return strlen(buf);
}