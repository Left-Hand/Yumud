#pragma once

#include <cstddef>
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"
#include "prelude.hpp"

namespace ymd::str{

struct [[nodiscard]] MutStrSpan final{
    char * begin;
    char * end;

    [[nodiscard]] constexpr size_t length() const {
        return static_cast<size_t>(end - begin);
    }
    __attribute__((optimize( "-Ofast" )))
    constexpr void fill(const char chr){
        for(char * p_chr = begin; p_chr < end; p_chr++){
            *p_chr = chr;
        }
    }
};

[[nodiscard]] static constexpr uint32_t  _div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = static_cast<uint32_t>((1ull << SHIFTS) / 100000 + 1);
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}



__attribute__((always_inline, hot, optimize( "-Ofast" )))
static constexpr size_t _least_u32_num_digits_dec(uint32_t int_val){
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

namespace details{
__attribute__((optimize( "-Ofast" )))
static constexpr void _fmtnum_u32_dec_inner(MutStrSpan s, uint32_t unsigned_val){
    // 从右向左填充数字
    char * pchr = s.end - 1;  // 当前填充位置

    {
        uint32_t quotient = str::div_10(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // 获取余数（即当前位数字）
    
        *pchr = digit + '0';                   // 转换为字符并填入字符串
        pchr--;
        if(pchr < s.begin) return;
        unsigned_val = quotient;                      // 处理下一位
    }

    // 从右到左逐位填充数字
    while (unsigned_val) {  // 当还有数字要处理且未越界时
        uint32_t quotient = str::div_10_maylossy(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // 获取余数（即当前位数字）

        *pchr = digit + '0';                   // 转换为字符并填入字符串
        pchr--;
        if(pchr < s.begin) return;
        unsigned_val = quotient;                      // 处理下一位
    }
}

}

__attribute__((optimize( "-Ofast" )))
[[nodiscard]] static constexpr char * _fmtnum_u32_dec_fittest(
    char * p_str, 
    uint32_t unsigned_val
) {
    if (unsigned_val == 0) {
        p_str[0] = '0';
        return p_str + 1;
    }

    const size_t len = _least_u32_num_digits_dec(unsigned_val);
    details::_fmtnum_u32_dec_inner({p_str, p_str + len}, unsigned_val);
    return p_str + len;
}

__attribute__((optimize( "-Ofast" )))
static constexpr void _fmtnum_u32_dec_padded(MutStrSpan s, uint32_t unsigned_val){
    // 即使数据为0也不做卫语句 会产生不必要的分支开销 对于小数位而言为0的可能性很小
    // 即使数据为0 也需要先填充所有位置为'0'

    s.fill('0');
    details::_fmtnum_u32_dec_inner(s, unsigned_val);
}


//TODO replace impl
static constexpr char * _stupid_fmtnum_u64_dec(char* p_str, uint64_t unsigned_val) {

    const size_t len = num_int2str_chars(static_cast<uint64_t>(unsigned_val), 10);
    int32_t i = len - 1;

    do {
		const uint8_t digit = unsigned_val % 10;
        p_str[i] = (digit) + '0';
        i--;
    } while((unsigned_val /= 10) > 0 and (i >= 0));

    return p_str + len;
}



}