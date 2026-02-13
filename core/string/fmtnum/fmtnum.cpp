#include "fmtnum.hpp"
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"

#include <array>

using namespace ymd;
using namespace ymd::str;


static constexpr char * _fmtnum_u32_r10(char* str, uint32_t unsigned_val) {
    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return str + 1;
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
        str[pos--] = digit + '0';              // Convert to character and place in string
        unsigned_val = quotient;                      // Move to next digit
    }

    return str + len;
}



static constexpr void _fmtnum_u32_r10_padded(char * str, uint32_t unsigned_val, const size_t len){
    // 即使数据为0 也需要先填充所有位置为'0'
    for (size_t i = 0; i < len; ++i) {
        str[i] = '0';
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
        str[pos--] = digit + '0';                   // 转换为字符并填入字符串
        unsigned_val = quotient;                      // 处理下一位
    }
    #else

    #endif
}


//TODO replace impl
static constexpr char * _stupid_fmtnum_u64_r10(char* str, uint64_t unsigned_val) {

    const size_t len = num_int2str_chars(static_cast<uint64_t>(unsigned_val), 10);
    int i = len - 1;

    do {
		const uint8_t digit = unsigned_val % 10;
        str[i] = (digit) + '0';
        i--;
    } while((unsigned_val /= 10) > 0 and (i >= 0));

    return str + len;
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


static constexpr char * _fmtnum_u32_r16(char* str, uint32_t unsigned_val) {
    const size_t len = u32_num_digits_r16((unsigned_val));
    int i = len - 1;

    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return str + 1;
    }

    // Convert number to hexadecimal string
    while (unsigned_val) {
        uint8_t digit = unsigned_val & 0b1111;  // Get lowest 4 bits (hex digit)
        str[i--] = digit > 9 ? (digit - 10 + 'A') : (digit + '0');
        unsigned_val >>= 4;                     // Move to next hex digit
    }

    return str + len;
}


// 使用 CLZ 计算 32 位无符号整数的八进制位数
static constexpr size_t _u32_num_digits_r8(uint32_t val) {
    if (val == 0) return 1;
    
    uint32_t bits_needed = 32 - __builtin_clz(val);  // 有效二进制位数
    
    // 八进制：每 3 位一个数字，向上取整
    return _div_3(bits_needed + 2);
}

// 测试用例
static_assert(_u32_num_digits_r8(0xFFFFFFFF) == 11);  // 37777777777 (32位全1，11位八进制)
static_assert(_u32_num_digits_r8(077777777) == 8);    // 8位八进制
static_assert(_u32_num_digits_r8(0777777) == 6);      // 6位八进制
static_assert(_u32_num_digits_r8(07777) == 4);        // 4位八进制
static_assert(_u32_num_digits_r8(077) == 2);          // 2位八进制
static_assert(_u32_num_digits_r8(07) == 1);           // 1位八进制
static_assert(_u32_num_digits_r8(0) == 1);            // 0特殊处理


static constexpr char * _fmtnum_u32_r8(char* str, uint32_t unsigned_val) {
    const size_t len = _u32_num_digits_r8(unsigned_val);
    int i = len - 1;

    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return str + 1;
    }

    // Convert number to octal string
    while (unsigned_val) {
        uint8_t digit = unsigned_val & 0b111;  // Get lowest 3 bits (octal digit)
        str[i--] = digit + '0';                // 八进制数字只能是0-7
        unsigned_val >>= 3;                    // Move to next octal digit
    }

    return str + len;
}


#if 1
// 使用 CLZ 计算二进制位数（保持与之前一致）
static constexpr size_t _u32_num_digits_r2(uint32_t val) {
    if (val == 0) return 1;
    return 32 - __builtin_clz(val);  // 或 std::countl_zero(val)
}

// 朴素二进制转换：每次处理1位，不使用查表，逻辑清晰
static constexpr char * _fmtnum_u32_r2(char* str, uint32_t unsigned_val) {
    // 处理 0 的特殊情况
    if (unsigned_val == 0) {
        str[0] = '0';
        return str + 1;
    }

    // 计算总位数
    const size_t total_len = _u32_num_digits_r2(unsigned_val);
    uint32_t pos = total_len;      // 从末尾开始填充
    uint32_t val = unsigned_val;

    // 逐位转换：从最低位开始，逆序填充
    while (val) {
        uint8_t digit = val & 1;   // 取最低位
        str[--pos] = digit + '0';  // 转为字符 '0' 或 '1'
        val >>= 1;                 // 右移处理下一位
    }

    // 此时 pos 应为 0，total_len 即为最终长度
    return str + total_len;
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

static constexpr size_t _fmtnum_u32_r2(uint32_t unsigned_val, char* str) {
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
constexpr char * _fmtnum_int_impl(char * str, T int_val, uint8_t radix){
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
                return _fmtnum_u32_r10(str, static_cast<uint32_t>(unsigned_val));
            }else{
                if(is_negative){
                    str[0] = '-';
                    str++;
                }

                if(unsigned_val <= 0xFFFFFFFFU){
                    return _fmtnum_u32_r10(str, static_cast<uint32_t>(unsigned_val));
                }

                return _stupid_fmtnum_u64_r10(str, unsigned_val);
            }
        case 16:
            return _fmtnum_u32_r16(str, unsigned_val);
            break;
        case 8:
            return _fmtnum_u32_r8(str, unsigned_val);
            break;
        case 2:
            return _fmtnum_u32_r2(str, unsigned_val);
        default:
            break;
    }

    //no chars 
    return str;

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


char * str::_fmtnum_signed_fixed_impl(
    char * str, 
    const int32_t value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    const bool is_negative = value_bits < 0;

	uint32_t abs_value_bits;
    if(is_negative){
        str[0] = '-';
        str++;
        // abs_value_bits = static_cast<unsigned_type>(-value_bits);
        abs_value_bits = static_cast<uint32_t>(-(value_bits + 1)) + 1;
    }else{
        abs_value_bits = static_cast<uint32_t>(value_bits);
    }
    return _fmtnum_unsigned_fixed_impl(str, abs_value_bits, precsion, Q);

}

char * str::_fmtnum_unsigned_fixed_impl(
    char * str, 
    uint32_t abs_value_bits, 
    uint8_t precsion, 
    const uint8_t Q
){
    // 安全限制precsion，确保不超出表格范围
    constexpr size_t MAX_PRECSION = std::size(pow10_table) - 1;
    if(precsion > MAX_PRECSION) precsion = MAX_PRECSION;

    const uint32_t LOWER_MASK = calc_low_mask(Q);
    const uint32_t ROUND_BIT = calc_low_mask(Q) & (~calc_low_mask(Q-1));
    const uint32_t POW10_SCALE = pow10_table[precsion];

    // 使用64位整数进行计算，避免溢出
    const uint64_t fs = static_cast<uint64_t>(abs_value_bits & LOWER_MASK) * POW10_SCALE;
    
    // 右移Q位提取小数部分（注意处理Q=0的情况）
    uint32_t frac_part;
    uint32_t digit_part;

    if(Q != 32)[[likely]]{
        // 计算舍入（基于小数部分的精度）
        const bool need_upper_round = (static_cast<uint32_t>(fs) & ROUND_BIT);

        frac_part = static_cast<uint32_t>(fs >> Q) + need_upper_round;

        // 检查是否需要进位到整数部分
        const bool carry_to_int = (frac_part >= POW10_SCALE);

        digit_part = (uint32_t(abs_value_bits) >> Q) + uint32_t(carry_to_int);

        // 如果发生进位，调整小数部分
        frac_part -= carry_to_int * POW10_SCALE;
    }else{
        frac_part = static_cast<uint32_t>(fs >> 32);
        digit_part = 0;
    }

    str = _fmtnum_u32_r10(str, digit_part);

    if(precsion){
        str[0] = '.';
        str++;
        _fmtnum_u32_r10_padded(str, frac_part, precsion);
        str += precsion;
    }

    return str;
}

char * str::fmtnum_i32(
    char *str, 
    int32_t int_val,
    uint8_t radix
){
    return _fmtnum_int_impl<int32_t>(str, int_val, radix);
}

char * str::fmtnum_u32(
    char *str, 
    uint32_t int_val,
    uint8_t radix
){
    return _fmtnum_int_impl<uint32_t>(str, int_val, radix);
}


char * str::fmtnum_u64(
    char *str,
    uint64_t int_val,
    uint8_t radix
){
    static constexpr uint64_t MASK = (~(uint64_t)std::numeric_limits<uint32_t>::max());
    const bool cant_be_represent_in_32 = int_val & MASK;
    if(cant_be_represent_in_32 == 0){
        return _fmtnum_int_impl<uint32_t>(str, int_val, radix);
    }

    //TODO 64位除法的实现会大幅增大体积
    return _fmtnum_int_impl<int64_t>(str, int_val, radix);
}


char * str::fmtnum_i64(
    char * str, 
    int64_t int_val, 
    uint8_t radix
){
    // return _fmtnum_int_impl<int64_t>(str, int_val, radix);
    return _fmtnum_int_impl<int32_t>(str, int_val, radix);
}


#if 0
[[maybe_unused]] static constexpr size_tfmtnum_ _f_impl(float value, char* str, uint8_t precision) {
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
    str += _fmtnum_u32_r10(static_cast<uint32_t>(int_part), str);
    
    // Convert fractional part
    if (precision > 0) {
        str[0] = '.';
        str++;
        _fmtnum_u32_r10_padded(frac_part, str, precision);
        str += precision;
    }
    
    return static_cast<size_t>(str - start);
}
#else

#if 1
[[maybe_unused]] static constexpr char * _fmtnum_f32_impl(
    char* str, 
    float value, 
    uint8_t precision
) {
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
        return str + 3;
    }
    
    if (is_negative) {
        str[0] = '-';
        str++;
    }

    // 检查无穷大
    if ((bits) == uint32_t(0x7F800000)) [[unlikely]] {
        str[0] = 'i'; str[1] = 'n'; str[2] = 'f';
        return str + 3;
    }

    // 处理零
    if (exponent == -127 && mantissa == 0) [[unlikely]] {
        str[0] = '0';
        return str + 1;
    }

    
    // 构建完整尾数（包含隐含的1）
    uint32_t full_mantissa = (mantissa | (1U << 23));
    
    // 计算精度缩放因子
    const uint32_t scale = pow10_table[precision];
    
    // 使用32位运算处理不同指数情况
    uint64_t scaled_value;
    if (exponent >= 0) {
        // 正指数处理

        // 乘以精度缩放因子
        scaled_value = static_cast<uint64_t>(full_mantissa) * scale;
        // 除以2^23 (通过右移)
        scaled_value >>= 23;
        // 应用正指数
        if (exponent > 0) {
            scaled_value <<= exponent;
        }
    } else {
        // 负指数处理

        // 乘以精度缩放因子
        scaled_value = static_cast<uint64_t>(full_mantissa) * scale;
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
    if (scaled_value < std::numeric_limits<uint32_t>::max()) {
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
    str = _fmtnum_u32_r10(str, int_part);
    
    // 转换小数部分
    if (precision > 0) {
        str[0] = '.';
        str++;
        _fmtnum_u32_r10_padded(str, frac_part, precision);
        str += precision;
    }
    
    return str;
}
#else
[[maybe_unused]] static constexpr size_tfmtnum_ _f_impl(float value, char* str, uint8_t precision) {
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
            if (scale <= std::numeric_limits<uint32_t>::max() / temp) {
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
                if (result <= std::numeric_limits<uint32_t>::max()) {
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
                if (scale <= std::numeric_limits<uint32_t>::max() / temp) {
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
                    
                    if (result <= std::numeric_limits<uint32_t>::max()) {
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
        str += _fmtnum_u32_r10(int_part, str);
    }
    
    // 输出小数部分
    if (precision > 0) {
        str[0] = '.';
        str++;
        _fmtnum_u32_r10_padded(frac_part, str, precision);
        str += precision;
    }
    
    return static_cast<size_t>(str - start);
}
#endif
#endif


char * str::fmtnum_f32(
    char * str, 
    float float_val, 
    uint8_t precision
){
    return _fmtnum_f32_impl(str, float_val, precision);
}
