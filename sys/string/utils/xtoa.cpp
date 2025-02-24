#include "StringUtils.hpp"
#include <array>

using namespace ymd;
using namespace ymd::StringUtils;


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