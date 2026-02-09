#include "xtoa.hpp"
#include "core/string/utils/pow10.hpp"
#include "core/string/utils/div10.hpp"
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





static constexpr size_t _u32toa_r10(uint32_t unsigned_val, char * str){

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

    if(unsigned_val == 0) return;
    
    auto fast_div10 = [](const uint32_t x) -> uint32_t{
        return str::div_10(x);
    };

    // 从右向左填充数字
    size_t pos = len - 1;  // 当前填充位置
    uint32_t temp_val = unsigned_val;  // 要转换的数值副本
    
    // 从右到左逐位填充数字
    while (temp_val > 0 && pos < len) {  // 当还有数字要处理且未越界时
        uint32_t quotient = fast_div10(temp_val);
        uint8_t digit = temp_val - quotient * 10;  // 获取余数（即当前位数字）
        str[pos] = digit + '0';                   // 转换为字符并填入字符串
        temp_val = quotient;                      // 处理下一位
        if (pos == 0) break;                      // 防止下标下溢
        --pos;
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


static constexpr size_t _uqtoa_impl(uint32_t abs_value_bits, char * const orignal_str, uint8_t eps, const uint8_t Q){

    // 安全限制eps，确保不超出表格范围
    constexpr size_t max_eps = std::size(pow10_table) - 1;
    eps = MIN(eps, static_cast<uint8_t>(max_eps));

    // 为任意Q生成掩码
    const uint64_t lower_mask = (Q >= 31) ? 
        0x7fffffffu :  // 对于Q31，特殊处理以避免左移32位
        ((Q == 0) ? 0 : ((1ULL << Q) - 1));

    const uint32_t frac_part = abs_value_bits & lower_mask;
    const uint32_t scale = pow10_table[eps];

    // 使用64位整数进行计算，避免溢出
    const uint64_t fs = (uint64_t)frac_part * scale;
    
    // 计算舍入（基于小数部分的精度）
    const bool need_upper_round = (fs & lower_mask) >= (lower_mask >> 1);

    // 右移Q位提取小数部分（注意处理Q=0的情况）
    const uint64_t shifted_fs = (Q == 0) ? fs : (fs >> Q);
    const uint64_t frac_int64 = shifted_fs + (need_upper_round ? 1 : 0);
    
    // 检查是否需要进位到整数部分
    const bool carry_to_int = (frac_int64 >= scale);
    const uint32_t digit_part = (uint32_t(abs_value_bits) >> Q) + (carry_to_int ? 1 : 0);
    
    // 如果发生进位，调整小数部分
    const uint32_t adjusted_frac_int = static_cast<uint32_t>(carry_to_int ? (frac_int64 - scale) : frac_int64);

    char * str = orignal_str;
    // str += _itoa_impl<uint32_t>(digit_part, str, 10);
    str += _u32toa_r10(digit_part, str);

    if(eps){
        str[0] = '.';
        str++;
        _u32toa_r10_padded(adjusted_frac_int, str, eps);
        str += eps;
    }

    return str - orignal_str;
}

size_t str::_uqtoa(const uint32_t abs_value_bits, char * str, uint8_t eps, const uint8_t Q){
    return _uqtoa_impl(abs_value_bits, str, eps, Q);
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