#pragma once

#include <cstdint>

namespace ymd::str{

// \frac{32\ln\left(2\right)}{\ln\left(10\right)} <= 10
static constexpr  uint32_t pow10_table[] = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
    1000000UL, 
    10000000UL, 
    100000000UL,
    1000000000UL
};

constexpr size_t num_int2str_chars(uint64_t int_val, const uint8_t radix){
    if(int_val == 0) return 1;

    size_t i = 0;
    uint64_t sum = 1;
    while(int_val >= sum){
        sum = sum * static_cast<uint64_t>(radix);
        i++;
    }
    return i > 0 ? i : 1;
}

template<typename T>
__always_inline constexpr T fast_div_5(T x){return (((int64_t)x*0x66666667L) >> 33);}

template<typename T>
__always_inline constexpr T fast_div_10(T x){(((int64_t)x*0x66666667L) >> 34);}

template<typename T>
requires(std::is_unsigned_v<T>)
__always_inline constexpr size_t uint_to_len_chars(T value, const uint8_t radix_count) {
    if (value == 0) return 1;

    // 优化 1: radix_count 是 2 的幂时，用位运算
    if ((radix_count & (radix_count - 1)) == 0) {
        const int log2_radix = __builtin_ctz(radix_count); // log2(radix_count)
        const int leading_zeros = __builtin_clz(value | 1); // 63 - bit_width(value)
        const int bit_width = 64 - leading_zeros;
        return (bit_width + log2_radix - 1) / log2_radix;
    }

    // 优化 2: 通用情况，用对数近似计算
    size_t length = 1;
    T sum = radix_count;
    while (value >= sum) {
        sum *= radix_count;
        length++;
    }
    return length;
}


}