#pragma once

#include "../real.hpp"
#include <cstdint>

struct fp16{
    union{
        uint16_t raw;
        struct{
            uint16_t frac:10;
            uint16_t exp:5;
            uint16_t sign:1;
        }__packed;
    }__packed;

    constexpr fp16(fixed_t qv):fp16(float(qv)){;}
    constexpr fp16(const fp16& other):raw(other.raw){;}
    constexpr fp16(float value) {

        // 下面的代码仅做示例，不保证正确性
        union {
            float input;
            uint32_t bits;
        } conversion;

        conversion.input = value;

        // 提取符号位
        sign = (conversion.bits >> 31) & 0x1;

        // 提取指数和尾数
        int exponent = ((conversion.bits >> 23) & 0xFF) - 127;
        uint32_t mantissa = conversion.bits & 0x007FFFFF;

        // 转换到fp16格式
        if (exponent > 30) { // 溢出处理
            exp = 0x1F;
            frac = 0;
        } else if (exponent <= -24) { // 下溢处理
            exp = 0;
            frac = 0;
        } else {
            // 移动指数和尾数
            exponent += 15;
            mantissa >>= (23 - (exponent - 15));
            exp = exponent;
            frac = mantissa;
        }

        raw = (sign << 15) | (exp << 10) | frac;
    }
    constexpr fp16(const int value){
        // 确保值在可表示的范围内
        if (value == 0) {
            exp = 0;
            frac = 0;
            sign = 0;
        } else if (value < 0) {
            sign = 1;
            value = -value;
        } else {
            sign = 0;
        }

        // fp16的指数范围是-14 ~ 15, 对于int值，我们假设它在[-32768, 32767]范围内
        // 这意味着我们最多有15位有效数字，这可以通过右移来适应fp16的10位小数部分

        // 计算指数
        int shift = 0;
        while (value >= (1 << 10)) {
            value >>= 1;
            shift++;
        }

        // 确保指数在fp16的范围内
        if (shift > 15) {
            exp = 0x1F; // 溢出
            frac = 0;
        } else if (shift < -14) {
            exp = 0; // 下溢
            frac = 0;
        } else {
            exp = shift + 15;
            frac = value;
        }

        raw = (sign << 15) | (exp << 10) | frac;
    }

    constexpr fp16(const double val):fp16((float)val){};

    explicit constexpr operator float() const {
        union {
            uint32_t raw;
            struct {
                uint32_t frac : 23;
                uint32_t exp : 8;
                uint32_t sign : 1;
            } __attribute__((__packed__));
        } conversion;

        // 从fp16的内部表示中提取符号、指数和尾数
        conversion.sign = sign;
        conversion.exp = exp + (127 - 15); // 调整指数偏移量
        conversion.frac = (frac << (23 - 10)); // 左移以填充更高位的0

        // 浮点数的隐含位
        conversion.frac |= (1 << 23);

        return *reinterpret_cast<const float*>(&conversion.raw);
    }

    explicit constexpr operator int() const {
        // 首先检查是否为NaN或无穷大
        if (exp == 0x1F && frac != 0) { // NaN
            return 0; // 或者可以选择抛出异常或返回特定值
        } else if (exp == 0x1F && frac == 0) { // 正无穷或负无穷
            return sign ? INT_MIN : INT_MAX;
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

    constexpr operator fixed_t() const{
        return fixed_t(float(*this));
    }
}__packed;