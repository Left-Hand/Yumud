#include "xtoa.hpp"
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"
#include "core/string/utils/reverse.hpp"
#include <array>

using namespace ymd;
using namespace ymd::str;


static constexpr size_t u32_num_digits_r10(uint32_t int_val){
    if(int_val == 0) return 1;

    size_t i = 0;
    uint64_t sum = 1;
    while(int_val >= sum){
        sum = sum * uint32_t(10);
        i++;
    }
    return i > 0 ? i : 1;
}

// 测试用例
static_assert(u32_num_digits_r10(0) == 1, "0 should return 1");
static_assert(u32_num_digits_r10(1) == 1, "1 should return 1");
static_assert(u32_num_digits_r10(9) == 1, "9 should return 1");
static_assert(u32_num_digits_r10(10) == 2, "10 should return 2");
static_assert(u32_num_digits_r10(99) == 2, "99 should return 2");
static_assert(u32_num_digits_r10(100) == 3, "100 should return 3");

// 关键测试：0x80000000
static_assert(u32_num_digits_r10(0x80000000) == 10, "0x80000000 should return 10");

// 更大值的测试
static_assert(u32_num_digits_r10(0xFFFFFFFF) == 10, "0xFFFFFFFF should return 10");
static_assert(u32_num_digits_r10(0x3B9ACA00) == 10, "0x3B9ACA00 (1e9) should return 10");
static_assert(u32_num_digits_r10(0x3B9ACA01) == 10, "0x3B9ACA01 should return 10");




__no_inline static constexpr size_t _u32toa_r10(uint32_t unsigned_val, char* str) {
    const size_t len = u32_num_digits_r10((unsigned_val));
    int i = len - 1;

    auto fast_div10 = [](const uint32_t x) -> uint32_t{
        return str::div_10(x);
    };

    // Handle special case of zero
    if (unsigned_val == 0) {
        str[0] = '0';
        return 1;
    }

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

    if(unsigned_val == 0) [[unlikely]] 
        return;
    
    auto fast_div10 = [](const uint32_t x) -> uint32_t{
        return str::div_10(x);
    };

    // 从右向左填充数字
    size_t pos = len - 1;  // 当前填充位置
    
    // 从右到左逐位填充数字

    while (unsigned_val > 0) {  // 当还有数字要处理且未越界时
        uint32_t quotient = fast_div10(unsigned_val);
        uint8_t digit = unsigned_val - quotient * 10;  // 获取余数（即当前位数字）
        str[pos] = digit + '0';                   // 转换为字符并填入字符串
        unsigned_val = quotient;                      // 处理下一位
        if (pos == 0) break;                      // 防止下标下溢
        --pos;
        if(pos >= len) __builtin_unreachable();
    }
}

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

    const size_t len = num_int2str_chars(static_cast<uint64_t>(unsigned_val), radix) + is_negative;
    int i = len - 1;

    do {
		const uint8_t digit = unsigned_val % radix;
        str[i] = ((digit) > 9) ? 
		(digit - 10) + ('A') : (digit) + '0';
        i--;
    } while((unsigned_val /= radix) > 0 and (i >= 0));

    if(is_negative) {
        str[0] = '-';
    }

    return len;
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