#pragma once

#include "decimal.hpp"
#include <span>

namespace ymd::str{




static constexpr size_t _least_u32_num_digits_hex(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // GCC/Clang 内置函数
    // 向上取整到 4 的倍数，再除以 4 得到十六进制位数
    return (bits_needed + 3) / 4;
}

static constexpr void _fmtnum_u32_hex(
    MutStrSpan s, 
    uint32_t unsigned_val
) {
    for(char * pchr = s.end - 1; pchr >= s.begin; --pchr){
        const uint8_t digit = unsigned_val & 0b1111;  // Get lowest 4 bits (hex digit)
        *pchr = digit > 9 ? (digit - 10 + 'A') : (digit + '0');
        unsigned_val >>= 4;                     // Move to next hex digit
    }
}

//n <= 34
static constexpr uint32_t _div_3(const uint32_t n){
    constexpr size_t SHIFTS = 32;
    constexpr uint32_t MAGIC = ((1ull << SHIFTS) / 3 + 1);
    return uint32_t((uint64_t(n) * MAGIC) >> SHIFTS);
}


// 使用 CLZ 计算 32 位无符号整数的八进制位数
static constexpr size_t _least_u32_num_digits_oct(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // 有效二进制位数
    
    // 八进制：每 3 位一个数字，向上取整
    return _div_3(bits_needed + 2);
}

template<typename T>
static constexpr size_t total_num_digits_oct(){
    switch(sizeof(T)){
        case 1: return _least_u32_num_digits_oct(0xff);
        case 2: return _least_u32_num_digits_oct(0xffff);
        case 4: return _least_u32_num_digits_oct(0xffff'ffff);
    }
    return 0;
}

static constexpr void _fmtnum_u32_oct(
    MutStrSpan s, 
    uint32_t unsigned_val
) {
    for(char * pchr = s.end - 1; pchr >= s.begin; --pchr){
        uint8_t digit = unsigned_val & 0b111;  // Get lowest 3 bits (octal digit)
        *pchr = digit + '0';                // 八进制数字只能是0-7
        unsigned_val >>= 3;                    // Move to next octal digit
    }

}



static constexpr void _fmtnum_u32_bin(
    MutStrSpan s, 
    uint32_t unsigned_val
) {
    if(s.length() < 32){
        unsigned_val = unsigned_val << (32 - s.length());
    }

    #pragma GCC unroll 4
    for(char * pchr = s.begin; pchr < s.end; ++pchr){
        *pchr = (unsigned_val & 0x80000000) ? '1' : '0';
        unsigned_val <<= 1;
    }
}

}