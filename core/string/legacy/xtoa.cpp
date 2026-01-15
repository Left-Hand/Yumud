#include "strconv.hpp"
#include <array>

using namespace ymd;
using namespace ymd::strconv;

static constexpr  uint32_t pow10_table[] = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
    1000000UL, 
    10000000UL, 
    
    // 100000000UL,
    // 1000000000UL
};


__fast_inline constexpr size_t _get_scalar(uint64_t value, const uint8_t radix){
    if(value == 0) return 1;

    size_t i = 0;
    uint64_t sum = 1;
    while(value >= sum){
        sum *= radix;
        i++;
    }
    return MAX(i, 1);
}



template<integral T>
static size_t _itoa_impl(T value, char * str, uint8_t radix){
    const bool is_negative = value < 0;
    if(is_negative) value = -value;

    const size_t len = _get_scalar(value, radix) + is_negative;
    str[len] = 0;
    int i = len - 1;

    do {
		const uint8_t digit = value % radix;
        str[i] = ((digit) > 9) ? 
		(digit - 10) + ('A') : (digit) + '0';
        i--;
    } while((value /= radix) > 0 and (i >= 0));

    if(is_negative) {
        str[0] = '-';
    }

    return len;
}


size_t strconv::_qtoa_impl(int32_t value_bits, char * str, uint8_t eps, const uint8_t Q){
    // 安全限制eps，确保不超出表格范围
    constexpr size_t max_eps = std::size(pow10_table) - 1;
    eps = MIN(eps, static_cast<uint8_t>(max_eps));

    const bool is_negative = value_bits < 0;
    const uint32_t abs_value = ABS(value_bits);
    
    // 为任意Q生成掩码
    const uint64_t lower_mask = (Q >= 31) ? 
        0x7fffffffu :  // 对于Q31，特殊处理以避免左移32位
        ((Q == 0) ? 0 : ((1ULL << Q) - 1));

    const uint32_t frac_part = uint32_t(abs_value) & lower_mask;
    const uint32_t scale = pow10_table[eps];

    // 使用64位整数进行计算，避免溢出
    const uint64_t fs = (uint64_t)frac_part * scale;
    
    // 计算舍入（基于小数部分的精度）
    const bool need_upper_round = (fs & lower_mask) >= (lower_mask >> 1);

    // 右移Q位提取小数部分（注意处理Q=0的情况）
    const uint64_t shifted_fs = (Q == 0) ? fs : (fs >> Q);
    uint64_t frac_int64 = shifted_fs + (need_upper_round ? 1 : 0);
    
    // 检查是否需要进位到整数部分
    const bool carry_to_int = (frac_int64 >= scale);
    const uint32_t int_part = (uint32_t(abs_value) >> Q) + (carry_to_int ? 1 : 0);
    
    // 如果发生进位，调整小数部分
    const uint64_t adjusted_frac_int64 = carry_to_int ? (frac_int64 - scale) : frac_int64;
    const uint32_t adjusted_frac_int = static_cast<uint32_t>(adjusted_frac_int64);

    size_t ind = 0;
    if(is_negative){
        str[0] = '-';
        ind++;
    }

    ind += _itoa_impl<int>(int_part, str + ind, 10);

    if(eps){
        str[ind] = '.';
        // 使用调整后的小数部分
        itoas(adjusted_frac_int, str + ind + 1, 10, eps);
    }

    return ind + (eps ? (1 + eps) : 0);
}

size_t strconv::itoa(int32_t value, char *str, uint8_t radix){
    return _itoa_impl<int32_t>(value, str, radix);
}


size_t strconv::iutoa(uint64_t value,char *str,uint8_t radix){
    // if(value > INT32_MAX or value < INT32_MIN){
    //     return _itoa_impl<int32_t>(value, str, radix);
    // }

    //TODO 64位除法的实现会大幅增大体积
    return _itoa_impl<int64_t>(value, str, radix);
}


size_t strconv::iltoa(int64_t value, char * str, uint8_t radix){
    // return _itoa_impl<int64_t>(value, str, radix);
    return _itoa_impl<int32_t>(value, str, radix);
}

#if 0
size_t strconv::ftoa(float number,char *buf, uint8_t eps)
{
    char str_int[12] = {0};
    char str_float[12] = {0};

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
#endif