#pragma once

#include "../real.hpp"
#include <cstdint>

namespace ymd::math{
struct alignas(2) [[nodiscard]] fp16 final{
    using Self = fp16;

    uint16_t frac:10;
    uint16_t exp:5;
    uint16_t sign:1;

    constexpr fp16() = default;

    template<size_t Q, typename D>
    constexpr fp16(fixed_t<Q, D> qv):fp16(float(qv)){;}
    constexpr fp16(const fp16& other) = default;

    [[nodiscard]] constexpr fp16 from_bits(const uint16_t bits){
        return std::bit_cast<fp16>(bits);
    }

    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }
    constexpr fp16(float f_val) {
        *this = f32_to_fp16(f_val);
    }

    constexpr fp16(int int_val){
        *this = int_to_fp16(int_val);
    }
    constexpr fp16(const double val):fp16(static_cast<float>(val)){};

    [[nodiscard]] constexpr bool is_nan() const {
        return to_bits() == 0x7fc0;
    }

    [[nodiscard]] explicit constexpr operator float() const {
        struct {
            uint32_t frac : 23;
            uint32_t exp : 8;
            uint32_t sign : 1;
        } conversion;

        // 从fp16的内部表示中提取符号、指数和尾数
        conversion.sign = sign;
        conversion.exp = exp + (127 - 15); // 调整指数偏移量
        conversion.frac = (frac << (23 - 10)); // 左移以填充更高位的0

        // 浮点数的隐含位
        conversion.frac |= (1 << 23);

        return std::bit_cast<float>(conversion);
    }

    template<typename D>
    requires (std::is_integral_v<D>)
    [[nodiscard]] explicit constexpr operator D() const {
        // 首先检查是否为NaN或无穷大
        if (exp == 0x1F && frac != 0) { // NaN
            return 0; // 或者可以选择抛出异常或返回特定值
        } else if (exp == 0x1F && frac == 0) { // 正无穷或负无穷
            return sign ? std::numeric_limits<D>::min() : std::numeric_limits<D>::max();
        }

        // 根据指数和尾数计算值
        int value = 0;
        if (exp != 0) {
            // 如果指数不为0，则计算2^exp * frac
            value = (1 << (exp - 15)) * frac;
        } else {
            // 如果指数为0，则检查是否为非规范数
            if (frac != 0) {
                // 非规范数，逐位相加直到最左边的1出现
                for (int i = 0; i < 10; i++) {
                    if (frac & (1 << i)) {
                        value = (1 << (-14 + i)) * (frac >> i);
                        break;
                    }
                }
            }
        }

        // 如果是负数，取反并减一得到补码表示
        if (sign) {
            value = -value;
        }

        return value;
    }


    template<size_t Q, typename D>
    [[nodiscard]] explicit constexpr operator fixed_t<Q, D>() const{
        return fixed_t<Q, D>(float(*this));
    }

private:
    static constexpr fp16 int_to_fp16(int int_val){
        fp16 ret = fp16();
        // 确保值在可表示的范围内
        if (int_val == 0) {
            ret.exp = 0;
            ret.frac = 0;
            ret.sign = 0;
        } else if (int_val < 0) {
            ret.sign = 1;
            int_val = -int_val;
        } else {
            ret.sign = 0;
        }

        // fp16的指数范围是-14 ~ 15, 对于int值，我们假设它在[-32768, 32767]范围内
        // 这意味着我们最多有15位有效数字，这可以通过右移来适应fp16的10位小数部分

        // 计算指数
        int shift = 0;
        while (int_val >= (1 << 10)) {
            int_val >>= 1;
            shift++;
        }

        // 确保指数在fp16的范围内
        if (shift > 15) {
            ret.exp = 0x1F; // 溢出
            ret.frac = 0;
        } else if (shift < -14) {
            ret.exp = 0; // 下溢
            ret.frac = 0;
        } else {
            ret.exp = shift + 15;
            ret.frac = int_val;
        }

        return ret;
    }

    static constexpr fp16 f32_to_fp16(const float f_val){
        fp16 ret;
        uint32_t bits = std::bit_cast<uint32_t>(f_val);

        // 提取符号位
        ret.sign = (bits >> 31) & 0x1;

        // 提取指数和尾数
        int exponent = ((bits >> 23) & 0xFF) - 127;
        uint32_t mantissa = bits & 0x007FFFFF;

        // 转换到fp16格式
        if (exponent > 30) { // 溢出处理
            ret.exp = 0x1F;
            ret.frac = 0;
        } else if (exponent <= -24) { // 下溢处理
            ret.exp = 0;
            ret.frac = 0;
        } else {
            // 移动指数和尾数
            exponent += 15;
            mantissa >>= (23 - (exponent - 15));
            ret.exp = exponent;
            ret.frac = mantissa;
        }

        return ret;
    }
    
};

static_assert(sizeof(fp16) == 2);

}