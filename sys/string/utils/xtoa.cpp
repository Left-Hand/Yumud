#include "StringUtils.hpp"
#include <array>

using namespace ymd;
using namespace ymd::StringUtils;

scexpr auto scale_map = [](){
    std::array<uint32_t, 8> ret = {};
    ret[0] = 1;
    for(size_t i = 1; i < ret.size(); i++){
        ret[i] = ret[i-1] * 10;
    }
    return ret;
}();



template<sintegral T>
static __fast_inline constexpr size_t get_scalar(T value){
    if(value == 0) return 1;

	size_t scalar = 0;
	value = ABS(value);
	
	while(value > 1000000){
		value /= 1000000;
		scalar += 6;
	}
	
	size_t i = 0;
	while(uint32_t(value) >= scale_map[i]) i++;
	return scalar + i;
}



template<uintegral T>
static __fast_inline constexpr size_t get_scalar(T value){
    if(value == 0) return 1;

	size_t scalar = 0;
	
	while(value > 1000000){
		value /= 1000000;
		scalar += 6;
	}
	
	size_t i = 0;

	while(value >= scale_map[i]) i++;
	return scalar + i;
}


// template<integral T>
// static void itoa_impl(T value, char * _str, uint8_t radix){
//     const bool minus = value < 0;
//     value = ABS(value);
//     const size_t scalar = get_scalar(value);
//     char * str = _str + scalar + minus - 1;
//     str[1] = 0;
//     do {
// 		const uint8_t digit = value % radix;
//         *str = ((digit) > 9) ? 
// 		(digit - 10) + ('A') : (digit) + '0';

//         str--;
//     } while((value /= radix) > 0 and (str >= _str));
//     // } while((value/=radix) and (str >= _str));
//     // }while(((value /= radix) > 0));

//     if(minus) {
//         _str[0] = '-';
//     }
// }

template<integral T>
static size_t itoa_impl(T value, char * str, uint8_t radix){
    value = ABS(value);
    const bool minus = value < 0;

    const size_t len = get_scalar(value) + minus;
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

// template<uintegral T>
// static void itoa_impl(T value, char * str, uint8_t radix){
//     int i = get_scalar(value) - 1;
//     str[i + 1] = 0;
//     do {
// 		const uint8_t digit = value % radix;
//         str[i] = ((digit) > 9) ? 
// 		(digit - 10) + ('A') : (digit) + '0';

//         i--;
//     } while((value /= radix) > 0 and (i >= 0));
// }


// template<integral T>
// static void itoa_impl(T value, char * str, uint8_t radix){
//     int sign = 0;
//     size_t i=0;
//     if(value < 0){
//         sign = -1;
//         value = -value;  
//     }
//     const size_t scalar = get_scalar(value);
//     do {
// 		const uint8_t digit = value % radix;
//         if(digit > 9){
//             str[i] = digit +'0'+7;
//         }else{
//             str[i] = digit +'0';
//         }i++;
//     } while((value/=radix)>0 and i <= scalar);
//     if(sign<0) {
//         str[i] = '-';
//         i++;
//     }

//     reverse_str(str, i);
// }


// template<integral T>
// static __fast_inline constexpr void itoa_impl(const T _value, char * _str, uint8_t radix){
// 	const bool minus = (_value < 0);
// 	std::make_unsigned_t<T> value = ABS(_value);
// 	const size_t scalar = get_scalar(_value);

// 	char * str = _str + scalar + minus - 1;
// 	do{
// 		const uint8_t digit = value % radix;
// 		*str = ((digit) > 9) ? 
// 		(digit - 10) + ('A') : (digit) + '0';
		
// 		str--;
// 	}while(((value /= radix) > 0) and (str >= _str));
	
// 	if(minus) {
// 		_str[0] = '-';
// 	}
// }

// static void itoas(int value, char *str, uint8_t radix, uint8_t size)  {
// 	size_t i = 0;
//     value = ABS(value);
// 	do{
// 		if(value%radix>9){
// 			str[i] = value % radix +'0' + 7;
//         }else{
// 			str[i] = value % radix +'0';
//         }
// 		i++;
// 	}while(((value /= radix) >0) && i < size);

// 	for(;i< size; i++)str[i] = '0';

// 	reverse_str(str, size);
// }

// static __fast_inline constexpr void itoas(uint32_t value, char *_str, uint8_t radix, uint8_t size)  {
// 	char * str = _str + size - 1;
// 	do{
// 		const uint8_t digit = value % radix;
// 		*str = ((digit) > 9) ? 
// 		(digit - 10) + ('A') : (digit) + '0';

// 		str--;
// 	}while(((value /= radix) > 0) and (str >= _str));
// 	// }while(((value /= radix) > 0));
	
// 	while(str >= _str){
// 		*str = '0';
// 		str--;
// 	}
// }

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

size_t StringUtils::qtoa(const iq_t value, char * str, uint8_t eps){

    eps = MIN(eps, 5);

	const bool minus = value < 0;
    const uint32_t abs_value = ABS(int32_t(_iq(value)));
    const uint32_t lower_mask = ((1 << GLOBAL_Q)- 1);

    const uint32_t frac_part = uint32_t(abs_value) & lower_mask;

    const uint32_t scale = scale_map[eps];

    const uint32_t fs = frac_part * scale;
    
    const bool upper_round = (fs & lower_mask) >= (lower_mask >> 1);

    const uint32_t frac_int = (fs >> GLOBAL_Q) + upper_round;
    const uint32_t int_part = (uint32_t(abs_value) >> GLOBAL_Q) + bool(frac_int >= scale);

    if(minus){
        str[0] = '-';
    }

    const auto end = itoa_impl<int>(int_part, str + minus, 10) + minus;

    if(eps){
        str[end] = '.';
        //add dot to seprate
        itoas(frac_int, str + end + 1, 10, eps);
    }

    return end + 1 + eps;
}



size_t StringUtils::itoa(int32_t value, char *str, uint8_t radix){
    return itoa_impl<int32_t>(value, str, radix);
}



size_t StringUtils::iutoa(uint64_t value,char *str,uint8_t radix){
    // int i=0;

    // do {
    //     if(value%radix>9)
    //         str[i] = value%radix +'0'+7;
    //     else
    //         str[i] = value%radix +'0';
    //     i++;
    // } while((value/=radix)>0);

    // reverse_str(str, i);
    return itoa_impl(value, str, radix);
}


size_t iltoa(int64_t value, char * str, uint8_t radix){
    return itoa_impl<int64_t>(value, str, radix);
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