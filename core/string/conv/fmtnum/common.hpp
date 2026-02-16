#pragma once

#include <cstddef>
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"

namespace ymd::str{
__attribute__((always_inline))

[[nodiscard]] static constexpr uint32_t  _div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = (1ull << SHIFTS) / 100000 + 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}


//Q = 0 is not granted
__attribute__((always_inline))
static constexpr uint32_t _calc_low_mask(const uint8_t Q){
    if (Q == 32) [[unlikely]] return 0xFFFFFFFFU;
    return (1U << Q) - 1;
}

static_assert(_calc_low_mask(31) == 0x7fffffffu);
static_assert(_calc_low_mask(32) == 0xffffffffu);
static_assert(_calc_low_mask(16) == 0x0000ffffu);

__attribute__((always_inline, hot))
static constexpr size_t _u32_num_digits_r10(uint32_t int_val){
    if(int_val == 0) [[unlikely]] return 1;

    auto match_result = [&](const uint32_t int_val_scaled) 
        -> size_t __attribute__((always_inline)
    ){
        if(int_val_scaled >= 100) [[likely]] {
            if(int_val_scaled >= 10000) return 5;
            else if(int_val_scaled >= 1000) return 4;
            else return 3;
        }else{
            if(int_val_scaled >= 10) return 2;
            else return 1;
        }
    };

    if(int_val >= 100000){
        int_val = _div_100000(int_val);
        return 5u + match_result(int_val);
    }else{
        return match_result(int_val);
    }

}

static constexpr char * _fmtnum_u32_r10(char* p_str, uint32_t unsigned_val) {
    // Handle special case of zero
    if (unsigned_val == 0) {
        p_str[0] = '0';
        return p_str + 1;
    }

    const size_t len = _u32_num_digits_r10((unsigned_val));
    size_t pos = len - 1;

    auto fast_div10 = [](const uint32_t x) -> uint32_t{
        return str::div_10(x);
    };

    // Convert number to string using fast division by 10
    while (unsigned_val) {
        uint32_t quotient = fast_div10(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // Get remainder (digit)
        p_str[pos--] = digit + '0';              // Convert to character and place in string
        unsigned_val = quotient;                      // Move to next digit
    }

    return p_str + len;
}



static constexpr void _fmtnum_u32_r10_padded(char * p_str, uint32_t unsigned_val, const size_t len){
    // 即使数据为0 也需要先填充所有位置为'0'
    for (size_t i = 0; i < len; ++i) {
        p_str[i] = '0';
    }

    // 即使数据为0也不做卫语句 会产生不必要的分支开销 对于小数位而言为0的可能性很小

    auto fast_div10 = [](const uint32_t x) -> uint32_t{
        #if 1
        return str::div_10(x);
        #else
        constexpr uint32_t MAGIC = ((1ull << 32) / 10u) + 1;
        return (uint64_t(x) * MAGIC) >> 32u;
        #endif
    };

    // 从右向左填充数字
    size_t pos = len - 1;  // 当前填充位置
    
    // 从右到左逐位填充数字

    #if 1
    while (unsigned_val) {  // 当还有数字要处理且未越界时
        #if 1
        uint32_t quotient = fast_div10(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // 获取余数（即当前位数字）
        #else
        uint32_t quotient = unsigned_val / 10;
        uint8_t digit = unsigned_val % 10;  // 获取余数（即当前位数字）
        #endif
        p_str[pos--] = digit + '0';                   // 转换为字符并填入字符串
        unsigned_val = quotient;                      // 处理下一位
    }
    #else

    #endif
}


//TODO replace impl
static constexpr char * _stupid_fmtnum_u64_r10(char* p_str, uint64_t unsigned_val) {

    const size_t len = num_int2str_chars(static_cast<uint64_t>(unsigned_val), 10);
    int i = len - 1;

    do {
		const uint8_t digit = unsigned_val % 10;
        p_str[i] = (digit) + '0';
        i--;
    } while((unsigned_val /= 10) > 0 and (i >= 0));

    return p_str + len;
}



}