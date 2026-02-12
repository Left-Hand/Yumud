#include "xtoa.hpp"
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"
#include "core/string/utils/reverse.hpp"
#include "core/utils/Result.hpp"
#include <array>

using namespace ymd;
using namespace ymd::str;


__attribute__((always_inline))

[[nodiscard]] static constexpr uint32_t  div_100000( const uint32_t u32_in ) noexcept{
    // constexpr size_t SHIFTS = 43;
    constexpr size_t SHIFTS = 48;
    constexpr uint32_t MAGIC = (1ull << SHIFTS) / 100000 + 1;
    return static_cast<uint32_t>((static_cast<uint64_t>(MAGIC) * (u32_in)) >> SHIFTS);
}

static_assert(div_100000(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 100000);




static constexpr size_t u32_num_digits_r10(uint32_t int_val){
    if(int_val == 0) [[unlikely]] return 1;
    size_t len = 0;
    if(int_val >= 100000){
        int_val = div_100000(int_val);
        len += 5;
    }


    if(int_val >= 100){
        if(int_val >= 10000) len += 5;
        else if(int_val >= 1000) len += 4;
        else len += 3;
    }else{
        if(int_val >= 10) len += 2;
        else len += 1;
    }

    return len;
}

// 测试用例
static_assert(u32_num_digits_r10(0) == 1, "0 should return 1");
static_assert(u32_num_digits_r10(1) == 1, "1 should return 1");
static_assert(u32_num_digits_r10(9) == 1, "9 should return 1");
static_assert(u32_num_digits_r10(10) == 2, "10 should return 2");
static_assert(u32_num_digits_r10(99) == 2, "99 should return 2");
static_assert(u32_num_digits_r10(100) == 3, "100 should return 3");
static_assert(u32_num_digits_r10(100000) == 6, "100 should return 3");

// 关键测试：0x80000000
static_assert(u32_num_digits_r10(0x80000000) == 10, "0x80000000 should return 10");

// 更大值的测试
static_assert(u32_num_digits_r10(0xFFFFFFFF) == 10, "0xFFFFFFFF should return 10");
static_assert(u32_num_digits_r10(0x3B9ACA00) == 10, "0x3B9ACA00 (1e9) should return 10");
static_assert(u32_num_digits_r10(0x3B9ACA01) == 10, "0x3B9ACA01 should return 10");

#if 0
static constexpr std::tuple<uint32_t, uint32_t> depart_hilo_18(const uint32_t hi, const uint32_t lo) {
    // 计算 val / 10^9 和 val % 10^9
    // val = hi * 2^32 + lo
    // 2^32 = 4294967296 = 4 * 10^9 + 294967296
    
    // 先处理高32位部分
    uint32_t hi_div_1e9 = hi / 1000000000;      // 高位贡献几个完整的10^9
    uint32_t hi_rem_1e9 = hi % 1000000000;      // 高位剩余部分
    
    // hi_rem_1e9 * 294967296 + lo 可能超过32位，用64位中间结果
    uint64_t carry = (uint64_t)hi_rem_1e9 * 294967296 + lo;
    
    // 合并贡献：
    // 1. hi_div_1e9 * 4 来自 hi_div_1e9 * (4 * 10^9) / 10^9
    // 2. carry / 10^9 来自剩余部分的贡献
    uint32_t quotient = hi_div_1e9 * 4 + (uint32_t)(carry / 1000000000);
    uint32_t remainder = (uint32_t)(carry % 1000000000);
    
    return {quotient, remainder};
}

// 基础测试
static_assert(depart_hilo_18(0, 0) == std::make_tuple(0u, 0u), "0 should return (0,0)");
static_assert(depart_hilo_18(0, 1) == std::make_tuple(0u, 1u), "1 should return (0,1)");
static_assert(depart_hilo_18(0, 9) == std::make_tuple(0u, 9u), "9 should return (0,9)");
static_assert(depart_hilo_18(0, 10) == std::make_tuple(0u, 10u), "10 should return (0,10)");
static_assert(depart_hilo_18(0, 999999999) == std::make_tuple(0u, 999999999u), "999999999 should return (0,999999999)");
static_assert(depart_hilo_18(0, 1000000000) == std::make_tuple(1u, 0u), "1e9 should return (1,0)");
static_assert(depart_hilo_18(0, 1000000001) == std::make_tuple(1u, 1u), "1e9+1 should return (1,1)");
static_assert(depart_hilo_18(0, 1999999999) == std::make_tuple(1u, 999999999u), "1999999999 should return (1,999999999)");
static_assert(depart_hilo_18(0, 2000000000) == std::make_tuple(2u, 0u), "2e9 should return (2,0)");
static_assert(depart_hilo_18(0, UINT32_MAX) == std::make_tuple(4u, 294967295u), "0xFFFFFFFF should return (4,294967295)");

// 测试高32位非零的情况
static_assert(depart_hilo_18(1, 0) == std::make_tuple(4u, 294967296u), "0x100000000 should return (4,294967296)");
static_assert(depart_hilo_18(1, 1) == std::make_tuple(4u, 294967297u), "0x100000001 should return (4,294967297)");
static_assert(depart_hilo_18(1, 4294967295) == std::make_tuple(5u, 294967295u), "0x1FFFFFFFF should return (5,294967295)");

// 测试边界值：2^32-1 和 2^32
static_assert(depart_hilo_18(0, 0xFFFFFFFF) == std::make_tuple(4u, 294967295u), "0xFFFFFFFF -> (4,294967295)");
static_assert(depart_hilo_18(1, 0) == std::make_tuple(4u, 294967296u), "0x100000000 -> (4,294967296)");

// 测试接近10^9整数倍的值
static_assert(depart_hilo_18(0, 4000000000) == std::make_tuple(4u, 0u), "4e9 should return (4,0)");
static_assert(depart_hilo_18(0, 4000000001) == std::make_tuple(4u, 1u), "4e9+1 should return (4,1)");
static_assert(depart_hilo_18(4, 294967296) == std::make_tuple(18u, 294967296u), "0x4FFFFFFFF? need verify");

// 测试最大值范围
static_assert(depart_hilo_18(0xFFFFFFFF, 0xFFFFFFFF) == 
              std::make_tuple(18446744073u, 554309260u), "0xFFFFFFFFFFFFFFFF should return known values");
#endif


static constexpr auto m_pow10 = [](size_t n) -> uint64_t {
    size_t sum = 1;
    for(size_t i = 0; i < n; i++){
        sum *= 10;
    }
    return sum;
};

[[maybe_unused]] void test_num_digits_r10(){
    constexpr auto u32_test_n = [&](size_t n) -> Result<void, void> {
        if(not (u32_num_digits_r10(m_pow10(n-1)) == n)) return Err();
        if(not (u32_num_digits_r10(m_pow10(n) - 1) == n)) return Err();
        return Ok();
    };

    constexpr auto test_all = [&]<typename Fn>(Fn && fn, size_t n) -> Result<void, int>{
        for(int i = 1; i <= int(n); i++){
            if(const auto res = (fn)(i); res.is_err()){
                return Err(i);
            }
        }
        return Ok();
    };

    static constexpr auto u32_res = test_all(u32_test_n, 9);
    static_assert(u32_res.is_ok(), "u32_num_digits_r10 failed");
}


static constexpr size_t _u32toa_r10(uint32_t unsigned_val, char* str) {
    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return 1;
    }

    const size_t len = u32_num_digits_r10((unsigned_val));
    int i = len - 1;

    auto fast_div10 = [](const uint32_t x) -> uint32_t{
        return str::div_10(x);
    };



    // Convert number to string using fast division by 10
    while (unsigned_val) {
        uint32_t quotient = fast_div10(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // Get remainder (digit)
        str[i--] = digit + '0';              // Convert to character and place in string
        unsigned_val = quotient;                      // Move to next digit
    }

    return len;
}



static constexpr void _u32toa_r10_padded(uint32_t unsigned_val, char * str, const size_t len){
    // 先填充所有位置为'0'
    for (size_t i = 0; i < len; ++i) {
        str[i] = '0';
    }

    if(unsigned_val == 0) [[unlikely]] {
        return;
    }

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

    while (unsigned_val > 0) {  // 当还有数字要处理且未越界时
        #if 1
        uint32_t quotient = fast_div10(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // 获取余数（即当前位数字）
        #else
        uint32_t quotient = unsigned_val / 10;
        uint8_t digit = unsigned_val % 10;  // 获取余数（即当前位数字）
        #endif
        str[pos] = digit + '0';                   // 转换为字符并填入字符串
        unsigned_val = quotient;                      // 处理下一位
        if (pos == 0) break;                      // 防止下标下溢
        --pos;
        if(pos >= len) __builtin_unreachable();
    }
}

static constexpr size_t _stupid_u64toa_r10(uint64_t unsigned_val, char* str) {

    const size_t len = num_int2str_chars(static_cast<uint64_t>(unsigned_val), 10);
    int i = len - 1;

    do {
		const uint8_t digit = unsigned_val % 10;
        str[i] = (digit) + '0';
        i--;
    } while((unsigned_val /= 10) > 0 and (i >= 0));

    return len;
}

static constexpr size_t u32_num_digits_r16(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // GCC/Clang 内置函数
    // 向上取整到 4 的倍数，再除以 4 得到十六进制位数
    return (bits_needed + 3) / 4;
}

static_assert(u32_num_digits_r16(0xFFFFFFFF) == 8);
static_assert(u32_num_digits_r16(0xFFFFFFF) == 7);
static_assert(u32_num_digits_r16(0xFFFFFF) == 6);
static_assert(u32_num_digits_r16(0xFFFFF) == 5);
static_assert(u32_num_digits_r16(0xFFFF) == 4);


static constexpr size_t _u32toa_r16(uint32_t unsigned_val, char* str) {
    const size_t len = u32_num_digits_r16((unsigned_val));
    int i = len - 1;

    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return 1;
    }

    // Convert number to hexadecimal string
    while (unsigned_val) {
        uint8_t digit = unsigned_val & 0b1111;  // Get lowest 4 bits (hex digit)
        str[i--] = digit > 9 ? (digit - 10 + 'A') : (digit + '0');
        unsigned_val >>= 4;                     // Move to next hex digit
    }

    return len;
}

//n <= 34
static constexpr uint32_t div_3(const uint32_t n){
    constexpr size_t SHIFTS = 32;
    constexpr uint32_t MAGIC = ((1ull << SHIFTS) / 3 + 1);
    return uint32_t((uint64_t(n) * MAGIC) >> SHIFTS);
}

static_assert(div_3(0) == 0);
static_assert(div_3(3) == 1);
static_assert(div_3(34) == 11);
static_assert(div_3(33) == 11);

// 使用 CLZ 计算 32 位无符号整数的八进制位数
static constexpr size_t u32_num_digits_r8(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // 有效二进制位数
    
    // 八进制：每 3 位一个数字，向上取整
    return div_3(bits_needed + 2);
}

// 测试用例
static_assert(u32_num_digits_r8(0xFFFFFFFF) == 11);  // 37777777777 (32位全1，11位八进制)
static_assert(u32_num_digits_r8(077777777) == 8);    // 8位八进制
static_assert(u32_num_digits_r8(0777777) == 6);      // 6位八进制
static_assert(u32_num_digits_r8(07777) == 4);        // 4位八进制
static_assert(u32_num_digits_r8(077) == 2);          // 2位八进制
static_assert(u32_num_digits_r8(07) == 1);           // 1位八进制
static_assert(u32_num_digits_r8(0) == 1);            // 0特殊处理


static constexpr size_t _u32toa_r8(uint32_t unsigned_val, char* str) {
    const size_t len = u32_num_digits_r8(unsigned_val);
    int i = len - 1;

    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return 1;
    }

    // Convert number to octal string
    while (unsigned_val) {
        uint8_t digit = unsigned_val & 0b111;  // Get lowest 3 bits (octal digit)
        str[i--] = digit + '0';                // 八进制数字只能是0-7
        unsigned_val >>= 3;                    // Move to next octal digit
    }

    return len;
}


#if 1
// 使用 CLZ 计算二进制位数（保持与之前一致）
static constexpr size_t u32_num_digits_r2(uint32_t val) {
    if (val == 0) return 1;
    return 32 - __builtin_clz(val);  // 或 std::countl_zero(val)
}

// 朴素二进制转换：每次处理1位，不使用查表，逻辑清晰
static constexpr size_t _u32toa_r2(uint32_t unsigned_val, char* str) {
    // 处理 0 的特殊情况
    if (unsigned_val == 0) {
        str[0] = '0';
        return 1;
    }

    // 计算总位数
    const size_t total_len = u32_num_digits_r2(unsigned_val);
    uint32_t pos = total_len;      // 从末尾开始填充
    uint32_t val = unsigned_val;

    // 逐位转换：从最低位开始，逆序填充
    while (val) {
        uint8_t digit = val & 1;   // 取最低位
        str[--pos] = digit + '0';  // 转为字符 '0' 或 '1'
        val >>= 1;                 // 右移处理下一位
    }

    // 此时 pos 应为 0，total_len 即为最终长度
    return total_len;
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

static constexpr size_t _u32toa_r2(uint32_t unsigned_val, char* str) {
    if (unsigned_val == 0) {
        str[0] = '0';
        return 1;
    }

    const size_t total_len = 32 - __builtin_clz(unsigned_val);
    uint32_t pos = total_len;          // 当前写入位置（从末尾向前移动）
    uint32_t val = unsigned_val;

    // 一次处理4位（低位组先写）
    for (; val >= 16; val >>= 4) {
        uint8_t nibble = val & 0xF;
        const char* chars = BIN_TABLE[nibble].data();
        str[--pos] = chars[3];        // 最低位
        str[--pos] = chars[2];
        str[--pos] = chars[1];
        str[--pos] = chars[0];        // 最高位
    }

    // 处理剩余不足4位的部分
    if (val > 0) {
        const char* chars = BIN_TABLE[val].data();
        uint32_t bits_remaining = pos;          // 剩余位数 = 有效位数
        uint32_t offset = 4 - bits_remaining;  // 跳过前导零

        for (uint32_t j = 0; j < bits_remaining; ++j) {
            str[--pos] = chars[offset + j];    // 从最高有效位开始取
        }
    }

    return total_len;
}
#endif

template<integral T>
static constexpr size_t _itoa_impl(T int_val, char * str, uint8_t radix){
    const bool is_negative = int_val < 0;
    std::make_unsigned_t<T> unsigned_val = [&]{
        if constexpr (std::is_signed_v<T>) {
            if(is_negative) {
                // 安全地取绝对值
                return static_cast<std::make_unsigned_t<T>>(-(int_val + 1)) + 1;
            } else {
                return static_cast<std::make_unsigned_t<T>>(int_val);
            }
        } else {
            return static_cast<std::make_unsigned_t<T>>(int_val);
        }
    }();

    switch(radix){
        case 10:
            static_assert(sizeof(T) <= 8);
            if constexpr(sizeof(T) <= 4){
                return _u32toa_r10(static_cast<uint32_t>(unsigned_val), str);
            }else{
                if(is_negative){
                    str[0] = '-';
                    str++;
                }

                size_t ind = is_negative;
                if(unsigned_val <= 0xFFFFFFFFU){
                    return ind + _u32toa_r10(unsigned_val, str);
                }

                return ind + _stupid_u64toa_r10(unsigned_val, str);
            }
        case 16:
            return _u32toa_r16(unsigned_val, str);
            break;
        case 8:
            return _u32toa_r8(unsigned_val, str);
            break;
        case 2:
            return _u32toa_r2(unsigned_val, str);
        default:
            break;
    }

    //no chars 
    return 0;

}

//Q = 0 is not granted
__attribute__((always_inline))
static constexpr uint32_t calc_low_mask(const uint8_t Q){
    if (Q == 0) __builtin_unreachable();
    if (Q == 32) [[unlikely]] return 0xFFFFFFFFU;
    return (1U << Q) - 1;
}

static_assert(calc_low_mask(31) == 0x7fffffffu);
static_assert(calc_low_mask(32) == 0xffffffffu);
static_assert(calc_low_mask(16) == 0x0000ffffu);


static constexpr size_t _uqtoa_impl(
    uint32_t abs_value_bits, 
    char * const orignal_str, 
    uint8_t precsion, 
    const uint8_t Q
){

    // 安全限制precsion，确保不超出表格范围
    constexpr size_t MAX_PRECSION = std::size(pow10_table) - 1;
    precsion = MIN(precsion, static_cast<uint8_t>(MAX_PRECSION));

    const uint32_t lower_mask = calc_low_mask(Q);
    const uint32_t frac_part = abs_value_bits & lower_mask;
    const uint32_t scale = pow10_table[precsion];

    // 使用64位整数进行计算，避免溢出
    const uint64_t fs = (uint64_t)frac_part * scale;
    
    // 计算舍入（基于小数部分的精度）
    const bool need_upper_round = (fs & lower_mask) >= (lower_mask >> 1);

    // 右移Q位提取小数部分（注意处理Q=0的情况）
    const uint64_t frac_u64 = (fs >> Q) + (need_upper_round ? 1 : 0);
    
    // 检查是否需要进位到整数部分
    const bool carry_to_int = (frac_u64 >= scale);
    const uint32_t digit_part = (uint32_t(abs_value_bits) >> Q) + (carry_to_int ? 1 : 0);
    
    // 如果发生进位，调整小数部分
    const uint32_t adjusted_frac_part = static_cast<uint32_t>(carry_to_int ? (frac_u64 - scale) : frac_u64);

    char * str = orignal_str;
    str += _u32toa_r10(digit_part, str);

    if(precsion){
        str[0] = '.';
        str++;
        _u32toa_r10_padded(adjusted_frac_part, str, precsion);
        str += precsion;
    }

    return str - orignal_str;
}

#if 0
[[maybe_unused]] static constexpr size_t _ftoa_impl(float value, char* str, uint8_t precision) {
    if (precision > 9) precision = 9;
    
    // Extract IEEE 754 floating point components
    uint32_t bits = std::bit_cast<uint32_t>(value);
    bool is_negative = (bits >> 31) != 0;
    int32_t exponent = ((bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = bits & 0x7FFFFF;
    bits &= 0x7FFFFFFF;
    
    // Check NaN
    if ((bits) > uint32_t(0x7F800000)) [[unlikely]] {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n';
        return 3;
    }
    
    // Check infinity
    if ((bits) == uint32_t(0x7F800000)) [[unlikely]] {
        if (is_negative) {
            str[0] = '-'; str[1] = 'i'; str[2] = 'n'; str[3] = 'f';
            return 4;
        } else {
            str[0] = 'i'; str[1] = 'n'; str[2] = 'f';
            return 3;
        }
    }

    // Handle zero
    if (exponent == -127 && mantissa == 0) [[unlikely]] {
        if (is_negative) {
            str[0] = '-'; str[1] = '0';
            return 2;
        } else {
            str[0] = '0';
            return 1;
        }
    }
    
    char* start = str;

    if (is_negative) {
        str[0] = '-';
        str++;
    }
    
    // Build complete mantissa (including implicit 1)
    uint64_t full_mantissa = (static_cast<uint64_t>(mantissa) | (1ULL << 23));
    
    // Calculate scaling factor for decimal precision
    uint64_t scale = pow10_table[precision];
    
    // Handle different exponent cases for proper conversion
    int64_t scaled_value;
    if (exponent >= 0) {
        // For positive exponents
        scaled_value = static_cast<int64_t>(full_mantissa);
        // Scale by 10^precision
        scaled_value *= static_cast<int64_t>(scale);
        // Divide by 2^23
        scaled_value >>= 23;
        // Apply positive exponent
        if (exponent > 0) {
            scaled_value <<= exponent;
        }
    } else {
        // For negative exponents
        scaled_value = static_cast<int64_t>(full_mantissa);
        // Scale by 10^precision
        scaled_value *= static_cast<int64_t>(scale);
        // Apply negative exponent (divide by 2^(-exponent))
        int shift_amount = 23 + (-exponent);
        if (shift_amount >= 0 && shift_amount < 64) {
            scaled_value >>= shift_amount;
        } else {
            scaled_value = 0;
        }
    }

    if (exponent >= 0) {
        // 计算余数用于判断舍入
        uint64_t unscaled = full_mantissa;
        unscaled <<= exponent;
        unscaled >>= 23;
    }
    
    // 判断是否需要进位（四舍五入）
    // 通过检查中间精度的小数部分来决定
    bool need_round_up = false;
    
    // 更精确的舍入判断
    if (exponent >= 0) {
        // 计算到更高精度来判断舍入
        uint64_t high_precision_scale = scale * 10; // 多一位精度
        uint64_t high_precision_value = full_mantissa;
        high_precision_value *= high_precision_scale;
        high_precision_value >>= 23;
        high_precision_value <<= exponent;
        
        // 获取额外的一位用于舍入判断
        uint64_t extra_digit = (high_precision_value / scale) % 10;
        need_round_up = (extra_digit >= 5);
    } else {
        // 对于负数指数，使用不同的舍入策略
        int shift_amount = 23 + (-exponent);
        uint64_t high_precision_scale = scale * 10;
        uint64_t high_precision_value = full_mantissa;
        high_precision_value *= high_precision_scale;
        high_precision_value >>= shift_amount;
        
        uint64_t extra_digit = (high_precision_value / scale) % 10;
        need_round_up = (extra_digit >= 5);
    }
    
    // 应用舍入
    if (need_round_up) {
        scaled_value += 1;
    }
    
    // Separate integer and fractional parts
    uint64_t int_part = scaled_value / scale;
    uint64_t frac_part = scaled_value % scale;
    
    // 处理进位导致整数部分增加的情况
    if (frac_part >= scale) {
        int_part += 1;
        frac_part -= scale;
    }
    
    // Convert integer part
    str += _u32toa_r10(static_cast<uint32_t>(int_part), str);
    
    // Convert fractional part
    if (precision > 0) {
        str[0] = '.';
        str++;
        _u32toa_r10_padded(frac_part, str, precision);
        str += precision;
    }
    
    return static_cast<size_t>(str - start);
}
#else

#if 1
[[maybe_unused]] static constexpr size_t _ftoa_impl(float value, char* str, uint8_t precision) {
    constexpr size_t MAX_PRECSION = std::size(pow10_table) - 1;
    if (precision > MAX_PRECSION) precision = MAX_PRECSION;
    
    
    // 提取IEEE 754浮点组件
    uint32_t bits = std::bit_cast<uint32_t>(value);
    bool is_negative = (bits >> 31) != 0;
    int32_t exponent = ((bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = bits & 0x7FFFFF;
    bits &= 0x7FFFFFFF;
    
    // 检查NaN
    if ((bits) > uint32_t(0x7F800000)) [[unlikely]] {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n';
        return 3;
    }
    
    // 检查无穷大
    if ((bits) == uint32_t(0x7F800000)) [[unlikely]] {
        if (is_negative) {
            str[0] = '-'; str[1] = 'i'; str[2] = 'n'; str[3] = 'f';
            return 4;
        } else {
            str[0] = 'i'; str[1] = 'n'; str[2] = 'f';
            return 3;
        }
    }

    // 处理零
    if (exponent == -127 && mantissa == 0) [[unlikely]] {
        if (is_negative) {
            str[0] = '-'; str[1] = '0';
            return 2;
        } else {
            str[0] = '0';
            return 1;
        }
    }
    
    char* start = str;

    if (is_negative) {
        str[0] = '-';
        str++;
    }
    
    // 构建完整尾数（包含隐含的1）
    uint32_t full_mantissa = (mantissa | (1U << 23));
    
    // 计算精度缩放因子
    const uint32_t scale = pow10_table[precision];
    
    // 使用32位运算处理不同指数情况
    uint64_t scaled_value;
    if (exponent >= 0) {
        // 正指数处理
        scaled_value = static_cast<uint64_t>(full_mantissa);
        // 乘以精度缩放因子
        scaled_value *= scale;
        // 除以2^23 (通过右移)
        scaled_value >>= 23;
        // 应用正指数
        if (exponent > 0) {
            scaled_value <<= exponent;
        }
    } else {
        // 负指数处理
        scaled_value = static_cast<uint64_t>(full_mantissa);
        // 乘以精度缩放因子
        scaled_value *= scale;
        // 应用负指数 (除以2^(-exponent))
        int shift_amount = 23 + (-exponent);
        if (shift_amount >= 0 && shift_amount < 64) {
            scaled_value >>= shift_amount;
        } else {
            scaled_value = 0;
        }
    }

    #if 1
    // 更精确的舍入判断，避免64位除法
    bool need_round_up = false;
    
    // 使用快速除法避免64位运算
    if (exponent >= 0) {
        // 计算更高精度来判断舍入
        uint64_t high_precision_scale = static_cast<uint64_t>(scale) * 10; // 多一位精度
        uint64_t high_precision_value = static_cast<uint64_t>(full_mantissa);
        high_precision_value *= high_precision_scale;
        high_precision_value >>= 23;
        high_precision_value <<= exponent;
        
        // 使用快速除法获取额外位用于舍入判断
        uint64_t temp_quotient = high_precision_value / scale;  // 这个除法可以用快速算法优化
        uint64_t extra_digit = temp_quotient % 10;
        need_round_up = (extra_digit >= 5);
    } else {
        int shift_amount = 23 + (-exponent);
        uint64_t high_precision_scale = static_cast<uint64_t>(scale) * 10;
        uint64_t high_precision_value = static_cast<uint64_t>(full_mantissa);
        high_precision_value *= high_precision_scale;
        high_precision_value >>= shift_amount;
        
        uint64_t temp_quotient = high_precision_value / scale;
        uint64_t extra_digit = temp_quotient % 10;
        need_round_up = (extra_digit >= 5);
    }
    
    // 应用舍入
    if (need_round_up) {
        scaled_value += 1;
    }
    #endif
    
    // 分离整数和小数部分，避免直接的64位除法
    uint32_t int_part, frac_part;
    
    // 使用预先计算的方法分离整数和小数部分
    if (scaled_value < UINT32_MAX) {
        // 对于较小的值，可以直接使用32位除法
        // 这里使用除法并不会比连续快速除法慢 除法占用约10个周期 快速除法占用约4个周期
        uint32_t val32 = static_cast<uint32_t>(scaled_value);
        int_part = val32 / scale;
        frac_part = val32 % scale;
    } else {
        // TODO 避免64位除法
        // 对于较大的值，使用快速分割算法
        // 避免直接的64位除法，使用查找表或近似算法
        
        // 使用二分查找或预计算的倒数来避免除法
        // 基于scale的值（来自pow10_table）我们可以使用专门的快速算法
        
        // 通用快速除法，针对pow10_table中的值进行优化
        int_part = static_cast<uint32_t>(scaled_value / scale);
        frac_part = static_cast<uint32_t>(scaled_value % scale);
    }
    
    // 检查进位到整数部分
    if (frac_part >= scale) {
        int_part += 1;
        frac_part -= scale;
    }
    
    // 转换整数部分
    str += _u32toa_r10(int_part, str);
    
    // 转换小数部分
    if (precision > 0) {
        str[0] = '.';
        str++;
        _u32toa_r10_padded(frac_part, str, precision);
        str += precision;
    }
    
    return static_cast<size_t>(str - start);
}
#else
[[maybe_unused]] static constexpr size_t _ftoa_impl(float value, char* str, uint8_t precision) {
    if (precision > 9) precision = 9;
    
    // 提取IEEE 754浮点组件
    uint32_t bits = std::bit_cast<uint32_t>(value);
    bool is_negative = (bits >> 31) != 0;
    int32_t exponent = ((bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = bits & 0x7FFFFF;
    bits &= 0x7FFFFFFF;
    
    // 检查特殊值
    if ((bits) > uint32_t(0x7F800000)) [[unlikely]] {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n';
        return 3;
    }
    
    if ((bits) == uint32_t(0x7F800000)) [[unlikely]] {
        if (is_negative) {
            str[0] = '-'; str[1] = 'i'; str[2] = 'n'; str[3] = 'f';
            return 4;
        } else {
            str[0] = 'i'; str[1] = 'n'; str[2] = 'f';
            return 3;
        }
    }

    if (exponent == -127 && mantissa == 0) [[unlikely]] {
        if (is_negative) {
            str[0] = '-'; str[1] = '0';
            return 2;
        } else {
            str[0] = '0';
            return 1;
        }
    }
    
    char* start = str;
    if (is_negative) {
        str[0] = '-';
        str++;
    }
    
    // 构建完整尾数（包含隐含的1）
    const uint32_t full_mantissa = (mantissa | (1U << 23));
    
    // 计算精度缩放因子
    const uint32_t scale = pow10_table[precision];
    
    // 根据指数分类处理
    uint32_t int_part = 0;
    uint32_t frac_part = 0;
    
    if (exponent >= 23) {
        // 纯整数部分
        int_part = full_mantissa;
        if (exponent > 23) {
            int_part <<= (exponent - 23);
        }
    } else if (exponent >= 0) {
        // 有整数和小数部分
        int_part = full_mantissa >> (23 - exponent);
        
        // 计算小数部分：使用快速算法
        const uint32_t frac_mantissa = full_mantissa & ((1U << (23 - exponent)) - 1);
        const uint32_t divisor_power = 23 - exponent;
        
        // 使用乘法代替除法：frac_mantissa * scale / 2^divisor_power
        // 我们将其转换为：(frac_mantissa * scale) >> divisor_power
        uint32_t temp = frac_mantissa;
        
        // 避免溢出，逐步计算
        if (temp != 0) {
            // 如果scale很小，可以直接计算
            if (scale <= UINT32_MAX / temp) {
                temp *= scale;
                temp >>= divisor_power;
                
                // 计算余数用于舍入
                uint32_t remainder = (frac_mantissa * scale) & ((1U << divisor_power) - 1);
                if (remainder >= (1U << (divisor_power - 1))) {
                    temp += 1;
                    if (temp >= scale) {
                        int_part += 1;
                        temp -= scale;
                    }
                }
                frac_part = temp;
            } else {
                // 需要更复杂的处理避免溢出
                // 使用分数形式进行计算
                uint64_t result = static_cast<uint64_t>(frac_mantissa) * scale;
                result >>= divisor_power;
                
                // 检查是否溢出，如果溢出则使用近似值
                if (result <= UINT32_MAX) {
                    uint32_t remainder = (frac_mantissa * scale) & ((1U << divisor_power) - 1);
                    if (remainder >= (1U << (divisor_power - 1))) {
                        result += 1;
                        if (result >= scale) {
                            int_part += 1;
                            result -= scale;
                        }
                    }
                    frac_part = static_cast<uint32_t>(result);
                }
            }
        }
    } else {
        // 小于1的数，纯小数
        int32_t effective_exp = -(exponent + 23);  // 负数表示需要左移多少位
        
        if (effective_exp >= 0) {
            // 数值很小，接近0
            frac_part = 0;
        } else {
            // effective_exp 是负数，所以 -effective_exp 是我们要右移的位数
            int32_t shift_right = -(effective_exp);
            
            if (shift_right <= 23) {
                // 计算小数部分：full_mantissa * scale / 2^shift_right
                uint32_t temp = full_mantissa;
                
                // 为了避免溢出，我们先尝试计算
                if (scale <= UINT32_MAX / temp) {
                    temp *= scale;
                    temp >>= shift_right;
                    
                    // 舍入处理
                    uint32_t remainder = (full_mantissa * scale) & ((1U << shift_right) - 1);
                    if (remainder >= (1U << (shift_right - 1))) {
                        temp += 1;
                        if (temp >= scale) {
                            int_part += 1;
                            temp -= scale;
                        }
                    }
                    frac_part = temp;
                } else {
                    // 溢出处理，使用64位临时计算
                    uint64_t result = static_cast<uint64_t>(full_mantissa) * scale;
                    result >>= shift_right;
                    
                    if (result <= UINT32_MAX) {
                        uint32_t remainder = (full_mantissa * scale) & ((1U << shift_right) - 1);
                        if (remainder >= (1U << (shift_right - 1))) {
                            result += 1;
                            if (result >= scale) {
                                int_part += 1;
                                result -= scale;
                            }
                        }
                        frac_part = static_cast<uint32_t>(result);
                    }
                }
            }
        }
    }
    
    // 输出整数部分
    if (int_part == 0) {
        str[0] = '0';
        str++;
    } else {
        str += _u32toa_r10(int_part, str);
    }
    
    // 输出小数部分
    if (precision > 0) {
        str[0] = '.';
        str++;
        _u32toa_r10_padded(frac_part, str, precision);
        str += precision;
    }
    
    return static_cast<size_t>(str - start);
}
#endif
#endif



size_t str::_uqtoa(const uint32_t abs_value_bits, char * str, uint8_t precsion, const uint8_t Q){
    return _uqtoa_impl(abs_value_bits, str, precsion, Q);
}


size_t str::itoa(int32_t int_val, char *str, uint8_t radix){
    return _itoa_impl<int32_t>(int_val, str, radix);
}


size_t str::iutoa(uint64_t int_val,char *str,uint8_t radix){
    static constexpr uint64_t MASK = (~(uint64_t)std::numeric_limits<uint32_t>::max());
    const bool cant_be_represent_in_32 = int_val & MASK;
    if(cant_be_represent_in_32 == 0){
        return _itoa_impl<uint32_t>(int_val, str, radix);
    }

    //TODO 64位除法的实现会大幅增大体积
    return _itoa_impl<int64_t>(int_val, str, radix);
}


size_t str::iltoa(int64_t int_val, char * str, uint8_t radix){
    // return _itoa_impl<int64_t>(int_val, str, radix);
    return _itoa_impl<int32_t>(int_val, str, radix);
}

size_t str::ftoa(float float_val, char * str, uint8_t precision){
    return _ftoa_impl(float_val, str, precision);
}