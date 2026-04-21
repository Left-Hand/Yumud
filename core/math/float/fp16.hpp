#pragma once

#include "core/math/real.hpp"
#include <cstdint>

namespace ymd::math{
struct alignas(2) [[nodiscard]] fp16 final{
    using Self = fp16;

    uint16_t frac:10;
    uint16_t exp:5;
    uint16_t sign:1;

    constexpr fp16() = default;

    template<size_t Q, typename D>
    constexpr fp16(fixed<Q, D> qv):fp16(float(qv)){;}
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
        *this = int32_to_fp16(int_val);
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
    [[nodiscard]] explicit constexpr operator fixed<Q, D>() const{
        return fixed<Q, D>(float(*this));
    }

private:

    static constexpr fp16 int32_to_fp16(int32_t int_val){
        fp16 ret = fp16();
        if (int_val == 0) {
            return ret;
        }

        if (int_val < 0) {
            ret.sign = 1;
            int_val = -int_val;
        }

        // 找到最高有效位的位置
        int exp = 0;
        uint32_t abs_val = int_val;
        while (abs_val > 0x7FF) { // 超过10位尾数 + 隐含整数位
            abs_val >>= 1;
            exp++;
        }

        if (exp > 15) { // 溢出
            ret.exp = 0x1F;
            ret.frac = 0;
        } else {
            ret.exp = exp + 15;
            ret.frac = abs_val & 0x3FF;
        }

        return ret;
    }

    static constexpr fp16 f32_to_fp16(const float f_val){
        fp16 ret = fp16();
        uint32_t bits = std::bit_cast<uint32_t>(f_val);

        // 提取符号位
        
        ret.sign = (bits >> 31) & 0x1;
// 提取指数和尾数
        int exponent = ((bits >> 23) & 0xFF) - 127;
        uint32_t mantissa = bits & 0x007FFFFF;

        // 转换到fp16格式
        if (exponent > 15) { // 溢出处理
            ret.exp = 0x1F;
            ret.frac = 0;
        } else if (exponent < -14) { // 下溢处理
            ret.exp = 0;
            ret.frac = 0;
        } else {
            // 调整指数：fp32的-127偏移改为fp16的15偏移
            ret.exp = exponent + 15;
            // 从23位尾数截断到10位尾数
            ret.frac = mantissa >> 13;
        }

        return ret;
    }
};

static_assert(sizeof(fp16) == 2);

}

namespace std{
	//建立元函数偏特化
    template<>
    struct is_arithmetic<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp16> : std::true_type {};
}
