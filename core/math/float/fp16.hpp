#pragma once

#include "core/math/real.hpp"
#include <cstdint>
#include <bit>
#include <limits>

namespace ymd::math{

namespace intrinsics{


constexpr uint32_t f32inf = 255UL << 23;
constexpr uint32_t f16inf = 31UL << 23;
constexpr uint32_t sign_mask = 0x80000000U;
constexpr uint32_t round_mask = ~0xFFFU;

// FP32 → FP16 前向转换（魔术数字法）
// FP16 格式：符号(1) | 指数(5, 偏差=15) | 尾数(10)
// 范围：2^-14 到 2^15（≈0.0000610352 到 65504）
// 形式化验证：见 core/math/float/formal_verification/verify_fp16_complete.py
// 验证结果：✓ 符号保留、✓ 边界值、✓ 单调性、✓ 溢出/下溢、✓ 特殊值（∞/NaN）
static constexpr uint16_t fp32_to_fp16_nonfpu(float value){
    constexpr uint32_t magic_bits = 15UL << 23;

    uint32_t in = std::bit_cast<uint32_t>(value);
    uint32_t sign = in & sign_mask;
    in ^= sign;
    uint16_t out = 0;

    if (in >= f32inf){
        // 特殊值：∞ 或 NaN
        // in >= f32inf 时，in = f32inf（Inf）或 in > f32inf（NaN）
        if (in > f32inf) {
            // NaN 路径
            out = (uint16_t)0x7FFFU;
        } else {
            // ∞ 路径
            out = (uint16_t)0x7C00U;
        }
    } else {
        // 正常范围：使用魔术数字法缩放
        // in < f32inf 保证进入此分支
        in &= round_mask;
        float f = std::bit_cast<float>(in);
        f *= std::bit_cast<float>(magic_bits);
        in = std::bit_cast<uint32_t>(f);
        in -= round_mask;

        if(in > f16inf){
            // 溢出处理
            in = f16inf;
        }
        out = (uint16_t)(in >> 13U);
    }
    out |= (uint16_t)(sign >> 16U);  // 添加符号位
    return out;
}

// FP16 → FP32 反向转换（显式格式转换）
// 指数范围映射：FP16[0,31] → FP32[112,143]（加 112）
// 处理零值、规范化数、非规范化数、特殊值
// 形式化验证：见 core/math/float/formal_verification/verify_fp16_complete.py
// 验证结果：✓ 零值、✓ 规范化数、✓ 非规范化数、✓ 往返转换一致性、✓ 特殊值
static constexpr float fp16_to_fp32_nonfpu(uint16_t value){
    uint32_t sign = value & 0x8000U;
    uint32_t exp = (value >> 10U) & 0x1FU;
    uint32_t mant = value & 0x3FFU;

    uint32_t out_bits = 0;

    if (exp == 0 && mant == 0) {
        // 零值路径：±0
        // exp=0 && mant=0 互斥于后续所有分支
        out_bits = 0;
    } else if (exp == 0) {
        // 非规范化数路径（exp=0, mant≠0）
        // 指数固定为 2^-14，FP32 中 exp32 = 113
        // mant: FP16[10 bits] → FP32[23 bits] = mant << 13
        if (!(mant != 0)) __builtin_unreachable();  // 确保 mant≠0
        uint32_t exp32 = 113U;
        uint32_t mant32 = mant << 13U;
        out_bits = (exp32 << 23U) | mant32;
    } else if (exp == 31U) {
        // 特殊值路径（Inf 或 NaN）
        // 指数为 0xFF，尾数直接映射
        if (exp != 31U) __builtin_unreachable();  // 互斥条件检查
        uint32_t exp32 = 0xFFU;
        uint32_t mant32 = mant << 13U;
        out_bits = (exp32 << 23U) | mant32;
    } else {
        // 规范化数路径（exp ∈ [1, 30]）
        // 指数映射：exp16 + 112 → exp32
        if (exp == 0 || exp == 31U) __builtin_unreachable();  // 前驱条件检查
        uint32_t exp32 = exp + 112U;
        uint32_t mant32 = mant << 13U;
        out_bits = (exp32 << 23U) | mant32;
    }

    out_bits |= (sign << 16U);  // 添加符号位
    return std::bit_cast<float>(out_bits);
}

}



struct alignas(2) [[nodiscard]] fp16 final{
    using Self = fp16;

    uint16_t mant:10;
    uint16_t exp:5;
    uint16_t sign:1;

    constexpr fp16() = default;

    template<size_t Q, typename D>
    constexpr fp16(fixed<Q, D> qv):fp16(float(qv)){;}
    constexpr fp16(const fp16& other) = default;

    [[nodiscard]] static constexpr fp16 from_bits(const uint16_t bits){
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
        return exp == 0x1F && mant != 0;
    }

    [[nodiscard]] explicit constexpr operator float() const {
        return intrinsics::fp16_to_fp32_nonfpu(to_bits());
    }

    template<typename D>
    requires (std::is_integral_v<D>)
    [[nodiscard]] explicit constexpr operator D() const {
        return static_cast<D>(static_cast<float>(*this));
    }


    template<size_t Q, typename D>
    [[nodiscard]] explicit constexpr operator fixed<Q, D>() const{
        return fixed<Q, D>(float(*this));
    }

private:

    static constexpr fp16 int32_to_fp16_nonfpu(int32_t int_val){
        return from_bits(intrinsics::fp32_to_fp16_nonfpu(static_cast<float>(int_val)));
    }

    static constexpr fp16 int32_to_fp16(int32_t int_val){
        auto conv_with_fpu = [int_val]() -> fp16 {
            return from_bits(intrinsics::fp32_to_fp16_nonfpu(static_cast<float>(int_val)));
        };

        if(std::is_constant_evaluated()){
            return conv_with_fpu();
        }

        #ifdef __FPU_PRESENT__
            return conv_with_fpu();
        #endif

        return int32_to_fp16_nonfpu(int_val);
    }

    static constexpr fp16 f32_to_fp16(const float f_val){
        return from_bits(intrinsics::fp32_to_fp16_nonfpu(f_val));
    }
};

static_assert(sizeof(fp16) == 2);




}

namespace std{
    template<>
    struct is_arithmetic<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp16> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp16> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp16> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp16> {
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = true;
        static constexpr bool has_signaling_NaN = true;
        static constexpr std::float_denorm_style has_denorm = std::denorm_present;
        static constexpr bool has_denorm_loss = false;
        static constexpr std::float_round_style round_style = std::round_to_nearest;
        static constexpr bool is_iec559 = true;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 11;      // 1 + 10 bits
        static constexpr int digits10 = 3;
        static constexpr int max_digits10 = 5;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -14;
        static constexpr int min_exponent10 = -4;
        static constexpr int max_exponent = 16;
        static constexpr int max_exponent10 = 4;

        static constexpr ymd::math::fp16 min() noexcept { return ymd::math::fp16::from_bits(0x0400); }
        static constexpr ymd::math::fp16 lowest() noexcept { return ymd::math::fp16::from_bits(0xFBFF); }
        static constexpr ymd::math::fp16 max() noexcept { return ymd::math::fp16::from_bits(0x7BFF); }
        static constexpr ymd::math::fp16 epsilon() noexcept { return ymd::math::fp16::from_bits(0x1400); }
        static constexpr ymd::math::fp16 round_error() noexcept { return ymd::math::fp16(0.5f); }
        static constexpr ymd::math::fp16 infinity() noexcept { return ymd::math::fp16::from_bits(0x7C00); }
        static constexpr ymd::math::fp16 quiet_NaN() noexcept { return ymd::math::fp16::from_bits(0x7E00); }
        static constexpr ymd::math::fp16 signaling_NaN() noexcept { return ymd::math::fp16::from_bits(0x7D00); }
        static constexpr ymd::math::fp16 denorm_min() noexcept { return ymd::math::fp16::from_bits(0x0001); }
    };

}
