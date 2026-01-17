#include "strconv.hpp"
#include <array>

using namespace ymd;
using namespace ymd::strconv;

static constexpr  uint32_t pow10_table[] = {
    1UL, 
    10UL, 
    100UL, 
    1000UL, 

    10000UL, 
    100000UL, 
    1000000UL, 
    10000000UL, 
    
    // 100000000UL,
    // 1000000000UL
};

constexpr size_t _get_scaler(uint64_t int_val, const uint8_t radix){
    if(int_val == 0) return 1;

    size_t i = 0;
    uint64_t sum = 1;
    while(int_val >= sum){
        sum = sum * static_cast<uint64_t>(radix);
        i++;
    }
    return i > 0 ? i : 1;
}

// 测试用例
static_assert(_get_scaler(0, 10) == 1, "0 should return 1");
static_assert(_get_scaler(1, 10) == 1, "1 should return 1");
static_assert(_get_scaler(9, 10) == 1, "9 should return 1");
static_assert(_get_scaler(10, 10) == 2, "10 should return 2");
static_assert(_get_scaler(99, 10) == 2, "99 should return 2");
static_assert(_get_scaler(100, 10) == 3, "100 should return 3");

// 关键测试：0x80000000
static_assert(_get_scaler(0x80000000, 10) == 10, "0x80000000 should return 10");

// 更大值的测试
static_assert(_get_scaler(0xFFFFFFFF, 10) == 10, "0xFFFFFFFF should return 10");
static_assert(_get_scaler(0x100000000, 10) == 10, "0x100000000 should return 10");
static_assert(_get_scaler(0x3B9ACA00, 10) == 10, "0x3B9ACA00 (1e9) should return 10");
static_assert(_get_scaler(0x3B9ACA01, 10) == 10, "0x3B9ACA01 should return 10");


template<integral T>
static size_t _itoa_impl(T int_val, char * str, uint8_t radix){
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

    const size_t len = _get_scaler(static_cast<uint64_t>(unsigned_val), radix) + is_negative;
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


size_t strconv::_qtoa_impl(int32_t value_bits, char * str, uint8_t eps, const uint8_t Q){
    // 安全限制eps，确保不超出表格范围
    constexpr size_t max_eps = std::size(pow10_table) - 1;
    eps = MIN(eps, static_cast<uint8_t>(max_eps));

    const bool is_negative = value_bits < 0;
    const uint32_t abs_value = ABS(value_bits);
    
    // 为任意Q生成掩码
    const uint64_t lower_mask = (Q >= 31) ? 
        0x7fffffffu :  // 对于Q31，特殊处理以避免左移32位
        ((Q == 0) ? 0 : ((1ULL << Q) - 1));

    const uint32_t frac_part = uint32_t(abs_value) & lower_mask;
    const uint32_t scale = pow10_table[eps];

    // 使用64位整数进行计算，避免溢出
    const uint64_t fs = (uint64_t)frac_part * scale;
    
    // 计算舍入（基于小数部分的精度）
    const bool need_upper_round = (fs & lower_mask) >= (lower_mask >> 1);

    // 右移Q位提取小数部分（注意处理Q=0的情况）
    const uint64_t shifted_fs = (Q == 0) ? fs : (fs >> Q);
    uint64_t frac_int64 = shifted_fs + (need_upper_round ? 1 : 0);
    
    // 检查是否需要进位到整数部分
    const bool carry_to_int = (frac_int64 >= scale);
    const uint32_t int_part = (uint32_t(abs_value) >> Q) + (carry_to_int ? 1 : 0);
    
    // 如果发生进位，调整小数部分
    const uint64_t adjusted_frac_int64 = carry_to_int ? (frac_int64 - scale) : frac_int64;
    const uint32_t adjusted_frac_int = static_cast<uint32_t>(adjusted_frac_int64);

    size_t ind = 0;
    if(is_negative){
        str[0] = '-';
        ind++;
    }

    ind += _itoa_impl<int32_t>(int_part, str + ind, 10);

    if(eps){
        str[ind] = '.';
        // 使用调整后的小数部分
        itoas(adjusted_frac_int, str + ind + 1, 10, eps);
    }

    return ind + (eps ? (1 + eps) : 0);
}

size_t strconv::itoa(int32_t int_val, char *str, uint8_t radix){
    return _itoa_impl<int32_t>(int_val, str, radix);
}


size_t strconv::iutoa(uint64_t int_val,char *str,uint8_t radix){
    // if(int_val > INT32_MAX or int_val < INT32_MIN){
    //     return _itoa_impl<int32_t>(int_val, str, radix);
    // }

    //TODO 64位除法的实现会大幅增大体积
    return _itoa_impl<int64_t>(int_val, str, radix);
}


size_t strconv::iltoa(int64_t int_val, char * str, uint8_t radix){
    // return _itoa_impl<int64_t>(int_val, str, radix);
    return _itoa_impl<int32_t>(int_val, str, radix);
}