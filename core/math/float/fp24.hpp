#pragma once

#include "fp32.hpp"
#include "fp64.hpp"
#include "core/math/real.hpp"

#include <cstdint>
#include <bit>
#include <array>
#include <span>

namespace ymd{
class OutputStream;
    
namespace math {

struct alignas(4) [[nodiscard]] fp24 final {
    // 内存布局：1位符号 + 8位指数 + 15位尾数
    // 24位存储在32位中，低8位未使用（便于对齐）
private:
    ::uint32_t padding   : 8;
public:
    ::uint32_t mantissa : 15;
    ::uint32_t exp      : 8;
    ::uint32_t sign     : 1;

    // 常量定义
    static constexpr ::uint32_t EXP_BIAS = 127;      // 与FP32相同的指数偏置
    static constexpr ::uint32_t EXP_MAX = 0xFF;      // 全1表示无穷大/NaN
    static constexpr ::uint32_t EXP_MIN = 0x00;      // 全0表示0或次正规数
    static constexpr ::uint32_t MANTISSA_BITS = 15;
    static constexpr ::uint32_t TOTAL_BITS = 24;
    static constexpr ::uint32_t STORAGE_BITS = 32;   // 实际存储大小

    constexpr fp24() : mantissa(0), exp(0), sign(0){}
    constexpr fp24(const fp24& other) = default;
    constexpr fp24(fp24&& other) = default;
    constexpr fp24& operator=(const fp24&) = default;
    constexpr fp24& operator=(fp24&&) = default;

    // 从float构造
    template<typename T>
    requires(std::is_floating_point_v<T>)
    constexpr fp24(T f_val) {
        *this = from_float(static_cast<float>(f_val));
    }

    // 从定点数构造
    template<size_t Q>
    constexpr fp24(fixed<Q, int32_t> qv) : fp24(float(qv)) {}

    // 从整数构造
    constexpr fp24(int int_val) : fp24(float(int_val)) {}
    constexpr fp24(unsigned int uint_val) : fp24(float(uint_val)) {}
    constexpr fp24(int64_t int_val) : fp24(float(int_val)) {}
    constexpr fp24(uint64_t uint_val) : fp24(float(uint_val)) {}

    // 一元负号运算符
    constexpr fp24 operator -() const {
        return fp24::from_bits(to_bits() ^ std::numeric_limits<uint32_t>::min());
    }

    // 从float创建fp24
    static constexpr fp24 from_float(float f) {
        uint32_t f32_bits = std::bit_cast<uint32_t>(f);
        return from_bits((f32_bits) >> 8);
    }

    // 从原始位创建
    static constexpr fp24 from_bits(uint32_t bits24) {
        return std::bit_cast<fp24>(bits24 << 8);
    }

    [[nodiscard]] constexpr uint32_t to_bits() const {
        return std::bit_cast<uint32_t>(*this) >> 8;
    }
    // fp24 -> float
    [[nodiscard]] explicit constexpr operator float() const {
        // 处理特殊情况
        if (exp == EXP_MAX) {  // 无穷大或NaN
            uint32_t f32_bits = (sign << 31) | (0xFF << 23);
            if (mantissa != 0) {  // NaN
                f32_bits |= 0x00400000;  // quiet NaN
            }
            return std::bit_cast<float>(f32_bits);
        }
        
        if (exp == EXP_MIN && mantissa == 0) {  // 零
            uint32_t f32_bits = (sign << 31);
            return std::bit_cast<float>(f32_bits);
        }
        
        // 正常或次正规数转换
        uint32_t f32_exp = exp;
        uint32_t f32_mantissa = static_cast<uint32_t>(mantissa) << 8;  // 15位→23位
        
        // 如果exp=0且mantissa≠0，是次正规数
        if (exp == EXP_MIN && mantissa != 0) {
            // 次正规数处理
            int32_t shift = __builtin_clz(mantissa) - 17;  // 15位数的前导零
            f32_exp = shift + 1;  // 调整指数
            f32_mantissa = (mantissa << (shift + 8)) & 0x7FFFFF;
        }
        
        uint32_t f32_bits = (sign << 31) | (f32_exp << 23) | f32_mantissa;
        return std::bit_cast<float>(f32_bits);
    }

    // fp24 -> int（截断）
    [[nodiscard]] explicit constexpr operator int() const {
        float f = static_cast<float>(*this);
        return static_cast<int>(f);
    }

    // fp24 -> 定点数
    template <size_t Q>
    [[nodiscard]] explicit constexpr operator fixed<Q, int32_t>() const {
        return fixed<Q, int32_t>::from(static_cast<float>(*this));
    }

    // 转换为3字节数组
    [[nodiscard]] constexpr std::array<uint8_t, 3> to_bytes() const {
        uint32_t bits = to_bits();
        std::array<uint8_t, 3> bytes;
        bytes[0] = (bits >> 16) & 0xFF;
        bytes[1] = (bits >> 8) & 0xFF;
        bytes[2] = bits & 0xFF;
        return bytes;
    }

    // 转换为字节视图
    [[nodiscard]] std::span<const uint8_t, 3> as_bytes() const {
        return std::span<const uint8_t, 3>(
            reinterpret_cast<const uint8_t*>(this) + 1// 跳过第一个填充字节
            , 3
        );
    }

    // 按指定字节序填充字节
    template<std::endian ENDIAN>
    constexpr void fill_bytes(std::span<uint8_t, 3> bytes) const {
        uint32_t bits = to_bits();
        if constexpr(ENDIAN == std::endian::little) {
            bytes[0] = bits & 0xFF;
            bytes[1] = (bits >> 8) & 0xFF;
            bytes[2] = (bits >> 16) & 0xFF;
        }else{
            bytes[0] = (bits >> 16) & 0xFF;
            bytes[1] = (bits >> 8) & 0xFF;
            bytes[2] = bits & 0xFF;
        }
    }


private:
};

static_assert(sizeof(fp24) == 4);

consteval fp24 operator"" _fp24(long double x){
    return fp24(x);
}

consteval fp24 operator"" _fp24(unsigned long long x){
    return fp24(x);
}

}

OutputStream & operator << (OutputStream & os, const math::fp24 v);
}


namespace std{

    template<>
    struct is_arithmetic<ymd::math::fp24> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp24> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp24> : std::true_type {};
}
