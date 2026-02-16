#pragma once

#include "common.hpp"


namespace ymd::str{
static constexpr size_t u32_num_digits_r16(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // GCC/Clang 内置函数
    // 向上取整到 4 的倍数，再除以 4 得到十六进制位数
    return (bits_needed + 3) / 4;
}

static constexpr char * _fmtnum_u32_r16(char* p_str, uint32_t unsigned_val) {
    const size_t len = u32_num_digits_r16((unsigned_val));
    int i = len - 1;

    // Handle special case of zero
    if (unsigned_val == 0) {
        p_str[0] = '0';
        return p_str + 1;
    }

    // Convert number to hexadecimal string
    while (unsigned_val) {
        uint8_t digit = unsigned_val & 0b1111;  // Get lowest 4 bits (hex digit)
        p_str[i--] = digit > 9 ? (digit - 10 + 'A') : (digit + '0');
        unsigned_val >>= 4;                     // Move to next hex digit
    }

    return p_str + len;
}

//n <= 34
static constexpr uint32_t _div_3(const uint32_t n){
    constexpr size_t SHIFTS = 32;
    constexpr uint32_t MAGIC = ((1ull << SHIFTS) / 3 + 1);
    return uint32_t((uint64_t(n) * MAGIC) >> SHIFTS);
}


// 使用 CLZ 计算 32 位无符号整数的八进制位数
static constexpr size_t _u32_num_digits_r8(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // 有效二进制位数
    
    // 八进制：每 3 位一个数字，向上取整
    return _div_3(bits_needed + 2);
}

static constexpr char * _fmtnum_u32_r8(char* p_str, uint32_t unsigned_val) {
    const size_t len = _u32_num_digits_r8(unsigned_val);
    int i = len - 1;

    // Handle special case of zero
    if (unsigned_val == 0) {
        p_str[0] = '0';
        return p_str + 1;
    }

    // Convert number to octal string
    while (unsigned_val) {
        uint8_t digit = unsigned_val & 0b111;  // Get lowest 3 bits (octal digit)
        p_str[i--] = digit + '0';                // 八进制数字只能是0-7
        unsigned_val >>= 3;                    // Move to next octal digit
    }

    return p_str + len;
}


#if 1
// 使用 CLZ 计算二进制位数（保持与之前一致）
static constexpr size_t _u32_num_digits_r2(uint32_t val) {
    if (val == 0) return 1;
    return 32 - __builtin_clz(val);  // 或 std::countl_zero(val)
}

// 朴素二进制转换：每次处理1位，不使用查表，逻辑清晰
static constexpr char * _fmtnum_u32_r2(char* p_str, uint32_t unsigned_val) {
    // 处理 0 的特殊情况
    if (unsigned_val == 0) {
        p_str[0] = '0';
        return p_str + 1;
    }

    // 计算总位数
    const size_t total_len = _u32_num_digits_r2(unsigned_val);
    uint32_t pos = total_len;      // 从末尾开始填充
    uint32_t val = unsigned_val;

    // 逐位转换：从最低位开始，逆序填充
    while (val) {
        uint8_t digit = val & 1;   // 取最低位
        p_str[--pos] = digit + '0';  // 转为字符 '0' 或 '1'
        val >>= 1;                 // 右移处理下一位
    }

    // 此时 pos 应为 0，total_len 即为最终长度
    return p_str + total_len;
}
#else

// 4位二进制到字符串的查找表
alignas(64) static constexpr std::array<std::array<char, 4>, 16> BIN_TABLE = []{
    std::array<std::array<char, 4>, 16> ret;
    for(size_t i = 0; i < 16; ++i){
        ret[i] = std::array<char, 4>{
            static_cast<char>(bool(i & (1 << 3)) + '0'), 
            static_cast<char>(bool(i & (1 << 2)) + '0'), 
            static_cast<char>(bool(i & (1 << 1)) + '0'), 
            static_cast<char>(bool(i & (1 << 0)) + '0'),
        };
    }
    return ret;
}();

static constexpr size_t _fmtnum_u32_r2(uint32_t unsigned_val, char* p_str) {
    if (unsigned_val == 0) {
        p_str[0] = '0';
        return 1;
    }

    const size_t total_len = 32 - __builtin_clz(unsigned_val);
    uint32_t pos = total_len;          // 当前写入位置（从末尾向前移动）
    uint32_t val = unsigned_val;

    // 一次处理4位（低位组先写）
    for (; val >= 16; val >>= 4) {
        uint8_t nibble = val & 0xF;
        const char* chars = BIN_TABLE[nibble].data();
        p_str[--pos] = chars[3];        // 最低位
        p_str[--pos] = chars[2];
        p_str[--pos] = chars[1];
        p_str[--pos] = chars[0];        // 最高位
    }

    // 处理剩余不足4位的部分
    if (val > 0) {
        const char* chars = BIN_TABLE[val].data();
        uint32_t bits_remaining = pos;          // 剩余位数 = 有效位数
        uint32_t offset = 4 - bits_remaining;  // 跳过前导零

        for (uint32_t j = 0; j < bits_remaining; ++j) {
            p_str[--pos] = chars[offset + j];    // 从最高有效位开始取
        }
    }

    return total_len;
}
#endif

}