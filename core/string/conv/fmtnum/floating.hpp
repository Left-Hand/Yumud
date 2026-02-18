#pragma once

#include "common.hpp"
#include "unsigned_fixed.hpp"

namespace ymd::str{

[[maybe_unused]] static constexpr char * _fmtnum_f32_impl(
    char* p_str, 
    float value, 
    uint8_t precision
) {
    constexpr size_t MAX_PRECSION = std::size(POW10_TABLE) - 1;
    if (precision > MAX_PRECSION) precision = MAX_PRECSION;
    
    
    // 提取IEEE 754浮点组件
    uint32_t bits = std::bit_cast<uint32_t>(value);
    bool is_negative = (bits >> 31) != 0;
    int32_t exponent = ((bits >> 23) & 0xFF) - 127;
    uint32_t mantissa = bits & 0x7FFFFF;
    bits &= 0x7FFFFFFF;
    
    // 检查NaN
    if ((bits) > uint32_t(0x7F800000)) [[unlikely]] {
        p_str[0] = 'n'; p_str[1] = 'a'; p_str[2] = 'n';
        return p_str + 3;
    }
    
    if (is_negative) {
        p_str[0] = '-';
        p_str++;
    }

    // 检查无穷大
    if ((bits) == uint32_t(0x7F800000)) [[unlikely]] {
        p_str[0] = 'i'; p_str[1] = 'n'; p_str[2] = 'f';
        return p_str + 3;
    }

    // 处理零
    if (exponent == -127 && mantissa == 0) [[unlikely]] {
        p_str[0] = '0';
        return p_str + 1;
    }

    
    // 构建完整尾数（包含隐含的1）
    uint32_t full_mantissa = (mantissa | (1U << 23));
    
    // 计算精度缩放因子
    const uint32_t scale = POW10_TABLE[precision];
    
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
    uint32_t digit_part, frac_part;
    
    // 使用预先计算的方法分离整数和小数部分
    if (scaled_value < UINT32_MAX) {
        // 对于较小的值，可以直接使用32位除法
        // 这里使用除法并不会比连续快速除法慢 除法占用约10个周期 快速除法占用约4个周期
        uint32_t val32 = static_cast<uint32_t>(scaled_value);
        digit_part = val32 / scale;
        frac_part = val32 % scale;
    } else {
        // TODO 避免64位除法
        // 对于较大的值，使用快速分割算法
        // 避免直接的64位除法，使用查找表或近似算法
        
        // 使用二分查找或预计算的倒数来避免除法
        // 基于scale的值（来自pow10_table）我们可以使用专门的快速算法
        
        // 通用快速除法，针对pow10_table中的值进行优化
        digit_part = static_cast<uint32_t>(scaled_value / scale);
        frac_part = static_cast<uint32_t>(scaled_value % scale);
    }
    
    // 检查进位到整数部分
    if (frac_part >= scale) {
        digit_part += 1;
        frac_part -= scale;
    }
    
    // 转换整数部分
    p_str = _fmtnum_u32_r10(p_str, digit_part);
    
    // 转换小数部分
    if (precision > 0) {
        p_str[0] = '.';
        p_str++;
        _fmtnum_u32_r10_padded(p_str, frac_part, precision);
        p_str += precision;
    }
    
    return p_str;
}
}