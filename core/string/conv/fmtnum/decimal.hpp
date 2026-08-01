#pragma once

#include <cstddef>
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"
#include "prelude.hpp"
#include <tuple>

namespace ymd::str{

struct [[nodiscard]] MutStrSpan final{
    char * begin;
    char * end;

    [[nodiscard]] constexpr size_t length() const noexcept {
        return static_cast<size_t>(end - begin);
    }
    __attribute__((optimize( "-Ofast" )))
    constexpr void fill(const char chr){
        // for(char * p_chr = begin; p_chr < end; p_chr++){
        //     *p_chr = chr;
        // }
        for(size_t i = 0; i < length(); i++){
            begin[i] = chr;
        }
    }
};

[[nodiscard]] static constexpr uint32_t  _div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = static_cast<uint32_t>((1ull << SHIFTS) / 100000 + 1);
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}


__attribute__((const, optimize("Ofast")))
static constexpr uint64_t mul64_hi(uint64_t a, uint64_t b) {
    uint32_t a0 = (uint32_t)a, a1 = (uint32_t)(a >> 32);
    uint32_t b0 = (uint32_t)b, b1 = (uint32_t)(b >> 32);
    uint64_t low  = (uint64_t)a0 * b0;
    uint64_t mid1 = (uint64_t)a0 * b1;
    uint64_t mid2 = (uint64_t)a1 * b0;
    uint64_t high = (uint64_t)a1 * b1;
    uint64_t carry = (low >> 32) + (mid1 & 0xFFFFFFFFULL) + (mid2 & 0xFFFFFFFFULL);
    return high + (mid1 >> 32) + (mid2 >> 32) + (carry >> 32);
}


__attribute__((const, optimize("Ofast")))
static constexpr std::tuple<uint64_t, uint32_t> bigint_divrem1e9(const uint64_t x) {

    constexpr uint32_t _1E9 = 1'000'000'000;

    // ceil(2^64 / 1e9)
    constexpr uint64_t MAGIC = (UINT64_MAX / uint64_t(_1E9)) + 1;

    if(x < _1E9) return {0, (uint32_t)x};

    uint64_t q = mul64_hi(x, MAGIC);   // 近似商，误差 ≤ 1

    if (q == 0) __builtin_unreachable();

    // 关键：计算 (q-1)*D，避免溢出（因为 (q-1)*D ≤ x）
    uint64_t tmp = x - (q - 1) * _1E9;     // 无溢出，tmp ∈ [0, 2D-1]

    if (tmp < uint64_t(_1E9)) {
        // q 偏大 1，真实商 = q-1
        return {q - 1, (uint32_t)tmp};
    } else {
        // q 即真实商，余数 = tmp - D
        return {q, (uint32_t)(tmp - _1E9)};
    }
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
static constexpr void _fmtnum_u32_dec_inner(MutStrSpan s, uint32_t abs_val){
    // 从右向左填充数字
    char * __restrict pchr = s.end - 1;  // 当前填充位置
    char * __restrict pbegin = s.begin;

    {
        uint32_t quotient = str::div_10(abs_val);
        uint8_t digit = abs_val - quotient * 10;  // 获取余数（即当前位数字）
    
        *pchr = digit + '0';                   // 转换为字符并填入字符串
        pchr--;
        if(pchr < pbegin) return;
        abs_val = quotient;                      // 处理下一位
    }

    // 从右到左逐位填充数字
    while (abs_val) {  // 当还有数字要处理且未越界时
        uint32_t quotient = str::div_10_maylossy(abs_val);
        uint8_t digit = abs_val - quotient * 10;  // 获取余数（即当前位数字）

        *pchr = digit + '0';                   // 转换为字符并填入字符串
        pchr--;
        if(pchr < pbegin) return;
        abs_val = quotient;                      // 处理下一位
    }
}

}

__attribute__((optimize( "-Ofast" )))
[[nodiscard]] static constexpr char * _fmtnum_u32_dec_fittest(
    char * __restrict p_str, 
    uint32_t abs_val
) {
    if (abs_val == 0) {
        p_str[0] = '0';
        return p_str + 1;
    }

    const size_t len = _least_u32_num_digits_dec(abs_val);
    details::_fmtnum_u32_dec_inner({p_str, p_str + len}, abs_val);
    return p_str + len;
}

__attribute__((optimize( "-Ofast" )))
static constexpr void _fmtnum_u32_dec_padded(MutStrSpan s, uint32_t abs_val){
    // 先填充所有位置为'0'
    s.fill('0');

    // 即使数据为0也不做卫语句 会产生不必要的分支开销 对于小数位而言为0的可能性很小
    details::_fmtnum_u32_dec_inner(s, abs_val);
}

//必须禁止此函数的循环展开，否则会严重膨胀，几次循环的性能开销很低
__attribute__((optimize("-Ofast", "no-unroll-loops")))
// __attribute__((optimize("-Ofast")))
[[nodiscard]] static constexpr char* _fmtnum_u64_dec_fittest(
    char * __restrict p_str,
    uint64_t abs_val
) {
    // 若高32位为0，直接委托给32位版本
    if (bool(abs_val >> 32) == false) {
        return _fmtnum_u32_dec_fittest(p_str, uint32_t(abs_val));
    }

    uint64_t quo = abs_val;
    uint32_t rem = 0;

    static constexpr size_t NUM_MAX_BLOCKS = 3;
    uint32_t blocks[NUM_MAX_BLOCKS];
    size_t cnt = 0;

    // 逐块分解，存储所有余数（低位块在前）
    while (true) {
        auto [q, r] = bigint_divrem1e9(quo);
        quo = q;
        rem = r;
        blocks[cnt++] = rem;
        if (quo == 0) break;
    }

    if(cnt > NUM_MAX_BLOCKS) __builtin_unreachable();

    // 从最高位块开始，向缓冲区正向写入
    for (int i = int(cnt - 1); i >= 0; --i) {
        uint32_t val = blocks[i];
        if (i == int(cnt - 1)) {
            // 最高位块：直接使用 u32 专用函数（自带位数计算）
            p_str = _fmtnum_u32_dec_fittest(p_str, val);
        } else {
            // 非最高位块：固定 9 位，左补零
            _fmtnum_u32_dec_padded({p_str, p_str + 9}, val);
            p_str += 9;
        }
    }
    return p_str;
}

}