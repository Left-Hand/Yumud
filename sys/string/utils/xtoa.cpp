#include "StringUtils.hpp"

using namespace yumud;

void StringUtils::qtoa(const iq_t value, char * str, uint8_t eps){

	bool minus = value < 0;
    eps = MIN(eps, 5);
    auto abs_value = ABS(int32_t(_iq(value)));

    uint32_t int_part = uint32_t(abs_value) >> GLOBAL_Q;
    uint32_t frac_part = uint32_t(abs_value) & ((1 << GLOBAL_Q )- 1);

    {
        if(minus){
            str[0] = '-';
        }

        StringUtils::itoa(int_part, str + minus, 10);
    }
	size_t end = strlen(str);

    if(eps){
		str[end] = '.';//add dot to seprate
		end += 1;//move to \0

        for(uint8_t i = 0; i < eps; i++){
            frac_part *= 10;
        }

        StringUtils::itoas(frac_part >> GLOBAL_Q,str + end, 10, eps);
    }
}

void StringUtils::itoa(int64_t value,char *str,uint8_t radix){
    int sign = 0;
    int i=0;
    if(value < 0){
        sign = -1;
        value = -value;  
    }
    do {
        if(value%radix>9)
            str[i] = value%radix +'0'+7;
        else
            str[i] = value%radix +'0';
        i++;
    } while((value/=radix)>0);
    if(sign<0) {
        str[i] = '-';
        i++;
    }

    reverse_str(str, i);
}

void StringUtils::itoas(int value,char *str,uint8_t radix, uint8_t size)  
{
	uint8_t i = 0;
    value = ABS(value);
	do{
		if(value%radix>9)
			str[i] = value%radix +'0'+7;
		else
			str[i] = value%radix +'0';
		i++;
	}while((value/=radix)>0 && i < size);
	for(;i< size; i++)str[i] = '0';

	reverse_str(str, size);
}

void StringUtils::iutoa(uint64_t value,char *str,uint8_t radix)
{
    int i=0;

    do {
        if(value%radix>9)
            str[i] = value%radix +'0'+7;
        else
            str[i] = value%radix +'0';
        i++;
    } while((value/=radix)>0);

    reverse_str(str, i);
}



void StringUtils::ftoa(float number,char *buf, uint8_t eps)
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
}