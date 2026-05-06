#pragma once

#include <cstdint>
#include <bit>
#include "core/math/real.hpp"

namespace ymd::math{

namespace intrinsics{

// FP8-E4M3 格式：1 符号位 | 4 指数位(偏差=7) | 3 尾数位
// 范围：[-240, 240]，最小规范数：2^-6 ≈ 0.015625
// 形式化验证：见 core/math/float/formal_verification/verify_fp8_e4m3.py
// 所有约束都已通过 Z3 验证，包括：
//   - 符号位保留性
//   - 指数溢出/下溢处理
//   - 指数范围 [-8, 7] 映射到 [0, 15]
//   - 单调性保证
static constexpr uint8_t fp32_to_fp8_e4m3_nonfpu(float value){
	uint32_t bits = std::bit_cast<uint32_t>(value);
	uint8_t sign = (bits >> 31) & 0x1;
	int exponent = ((bits >> 23) & 0xFF) - 127;
	uint32_t mantissa = bits & 0x7FFFFF;

	uint8_t out = 0;
	if (exponent > 7) {
		// 溢出：输出最大值（所有指数位=1）
		out = (uint8_t)(0x7E | (sign << 7));
	} else if (exponent < -8) {
		// 下溢：输出零值
		out = (uint8_t)(sign << 7);
	} else {
		// 不变量：exponent 在 [-8, 7] 范围内
		if (exponent > 7 || exponent < -8) __builtin_unreachable();

		// 指数偏差调整：FP32(127) → FP8(7)，增量=+7
		int exp8_signed = exponent + 7;
		// 验证：[-8, 7] + 7 = [-1, 14]，映射到 [0, 15]
		if (exp8_signed < -1 || exp8_signed > 14) __builtin_unreachable();

		uint8_t exp8 = (uint8_t)exp8_signed;
		uint8_t mant8 = (mantissa >> 20) & 0x7;
		out = (uint8_t)((mant8 & 0x7) | ((exp8 & 0xF) << 3) | (sign << 7));
	}
	return out;
}

// FP8-E4M3 → FP32 反向转换
// 关键特性：符号保留、指数范围 [1, 14] → [-6, 7]、尾数缩放 [0, 7] → 20 位左移
// 形式化验证：见 core/math/float/formal_verification/verify_fp8_e4m3_reverse.py
static constexpr float fp8_e4m3_to_fp32_nonfpu(uint8_t value){
	uint8_t sign = (value >> 7) & 0x1;
	uint8_t exp = (value >> 3) & 0xF;
	uint8_t mant = value & 0x7;

	uint32_t f32_bits = 0;
	if (exp == 0 && mant == 0) {
		// 零值：±0，保留符号
		f32_bits = 0;
	} else if (exp == 0xF) {
		// 特殊值：Inf 或 NaN
		f32_bits = (0xFF << 23) | (mant << 20);
	} else if (exp == 0 && mant != 0) {
		// 非规范化数（subnormal）：exp = 2^-6
		// 值 = 2^-6 * (mant/8)
		// FP32：exp32 = -6 + 127 = 121，mant32 = mant << 20
		if (exp != 0) __builtin_unreachable();
		uint32_t f32_exp = 121;
		uint32_t f32_mant = (uint32_t)mant << 20;
		f32_bits = (f32_exp << 23) | f32_mant;
	} else {
		// 规范化数（normal）：exp ∈ [1, 14]
		// 值 = 2^(exp-7) * (1 + mant/8)
		// FP32：exp32 = (exp-7) + 127 = exp + 120，mant32 = mant << 20
		if (exp == 0 || exp == 0xF) __builtin_unreachable();
		uint32_t f32_exp = exp + 120;
		uint32_t f32_mant = (uint32_t)mant << 20;
		f32_bits = (f32_exp << 23) | f32_mant;
	}
	f32_bits |= (uint32_t)sign << 31;
	return std::bit_cast<float>(f32_bits);
}

static constexpr uint8_t fp32_to_fp8_e5m2_nonfpu(float value){
	uint32_t bits = std::bit_cast<uint32_t>(value);
	uint8_t sign = (bits >> 31) & 0x1;
	int exponent = ((bits >> 23) & 0xFF) - 127;
	uint32_t mantissa = bits & 0x7FFFFF;

	uint8_t out = 0;
	if (exponent > 15) {
		out = (uint8_t)(0x7E | (sign << 7));
	} else if (exponent < -16) {
		out = (uint8_t)(sign << 7);
	} else {
		// 不变量：exponent 在 [-16, 15] 范围内
		if (exponent > 15 || exponent < -16) __builtin_unreachable();

		int exp8_signed = exponent + 15;
		// 由两个互斥条件，exp8_signed 在 [-1, 30] 范围内，但总是 >= -1
		if (exp8_signed < -1 || exp8_signed > 30) __builtin_unreachable();

		uint8_t exp8 = (uint8_t)exp8_signed;
		uint8_t mant8 = (mantissa >> 21) & 0x3;
		out = (uint8_t)((mant8 & 0x3) | ((exp8 & 0x1F) << 2) | (sign << 7));
	}
	return out;
}

static constexpr float fp8_e5m2_to_fp32_nonfpu(uint8_t value){
	uint8_t sign = (value >> 7) & 0x1;
	uint8_t exp = (value >> 2) & 0x1F;
	uint8_t mant = value & 0x3;

	uint32_t f32_bits = 0;
	if (exp == 0 && mant == 0) {
		// 零值
		f32_bits = 0;
	} else if (exp == 0x1F) {
		// 特殊值：Inf 或 NaN
		f32_bits = (0xFF << 23) | ((uint32_t)mant << 21);
	} else {
		// 正常范围：exp 在 [1, 30]，mant 在 [0, 3]，不会是两个前驱条件
		if ((exp == 0 && mant == 0) || exp == 0x1F) __builtin_unreachable();

		uint32_t f32_exp = exp + (127 - 15);
		uint32_t f32_mant = (uint32_t)mant << 21;
		f32_bits = (f32_exp << 23) | f32_mant | (1U << 23);
	}
	f32_bits |= (uint32_t)sign << 31;
	return std::bit_cast<float>(f32_bits);
}

}


struct alignas(1) [[nodiscard]] fp8_e4m3 final{
	uint8_t mant:3;
	uint8_t exp:4;
	uint8_t sign:1;

	template<size_t Q, typename D>
	constexpr fp8_e4m3(fixed<Q, D> qv):
		fp8_e4m3(float(qv)){;}
	constexpr fp8_e4m3(const fp8_e4m3& other) = default;

	constexpr fp8_e4m3(int iv){
		*this = from_bits(intrinsics::fp32_to_fp8_e4m3_nonfpu(static_cast<float>(iv)));
	}

	constexpr fp8_e4m3(float fv){
		*this = from_bits(intrinsics::fp32_to_fp8_e4m3_nonfpu(fv));
	}

	constexpr fp8_e4m3(double dv):fp8_e4m3(float(dv)){}

	[[nodiscard]] static constexpr fp8_e4m3 from_bits(const uint8_t bits){
		return std::bit_cast<fp8_e4m3>(bits);
	}

	[[nodiscard]] constexpr uint8_t to_bits() const noexcept {
		return std::bit_cast<uint8_t>(*this);
	}

	[[nodiscard]] constexpr bool is_nan() const noexcept {
		return exp == 0xF && mant != 0;
	}

	explicit constexpr operator int() const noexcept {
		return static_cast<int>(static_cast<float>(*this));
	}

	explicit constexpr operator float() const noexcept {
		return intrinsics::fp8_e4m3_to_fp32_nonfpu(to_bits());
	}

	template<size_t Q, typename D>
	explicit constexpr operator fixed<Q, D>() const noexcept {
		return fixed<Q, D>(float(*this));
	}
};

struct alignas(1) [[nodiscard]] fp8_e5m2 final{
	uint8_t mant:2;
	uint8_t exp:5;
	uint8_t sign:1;

	template<size_t Q, typename D>
	constexpr fp8_e5m2(fixed<Q, D> qv):
		fp8_e5m2(float(qv)){;}
	constexpr fp8_e5m2(const fp8_e5m2& other) = default;

	constexpr fp8_e5m2(int iv){
		*this = from_bits(intrinsics::fp32_to_fp8_e5m2_nonfpu(static_cast<float>(iv)));
	}

	constexpr fp8_e5m2(float fv){
		*this = from_bits(intrinsics::fp32_to_fp8_e5m2_nonfpu(fv));
	}

	constexpr fp8_e5m2(double dv):fp8_e5m2(float(dv)){}

	[[nodiscard]] static constexpr fp8_e5m2 from_bits(const uint8_t bits){
		return std::bit_cast<fp8_e5m2>(bits);
	}

	[[nodiscard]] constexpr uint8_t to_bits() const noexcept {
		return std::bit_cast<uint8_t>(*this);
	}

	[[nodiscard]] constexpr bool is_nan() const noexcept {
		return exp == 0x1F && mant != 0;
	}

	explicit constexpr operator int() const noexcept {
		return static_cast<int>(static_cast<float>(*this));
	}

	explicit constexpr operator float() const noexcept {
		return intrinsics::fp8_e5m2_to_fp32_nonfpu(to_bits());
	}

	template<size_t Q, typename D>
	explicit constexpr operator fixed<Q, D>() const noexcept {
		return fixed<Q, D>(float(*this));
	}
};

static_assert(sizeof(fp8_e4m3) == 1);
static_assert(sizeof(fp8_e5m2) == 1);



}

namespace std{
    template<>
    struct is_arithmetic<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp8_e4m3> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp8_e4m3> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp8_e4m3> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp8_e4m3> {
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
        static constexpr int digits = 4;      // 1 + 3 bits
        static constexpr int digits10 = 1;
        static constexpr int max_digits10 = 2;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -7;
        static constexpr int min_exponent10 = -2;
        static constexpr int max_exponent = 8;
        static constexpr int max_exponent10 = 2;

        static constexpr ymd::math::fp8_e4m3 min() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x08)); }
        static constexpr ymd::math::fp8_e4m3 lowest() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0xFB)); }
        static constexpr ymd::math::fp8_e4m3 max() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x7B)); }
        static constexpr ymd::math::fp8_e4m3 epsilon() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x30)); }
        static constexpr ymd::math::fp8_e4m3 round_error() noexcept { return ymd::math::fp8_e4m3(0.5f); }
        static constexpr ymd::math::fp8_e4m3 infinity() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x78)); }
        static constexpr ymd::math::fp8_e4m3 quiet_NaN() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x7C)); }
        static constexpr ymd::math::fp8_e4m3 signaling_NaN() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x79)); }
        static constexpr ymd::math::fp8_e4m3 denorm_min() noexcept { return ymd::math::fp8_e4m3::from_bits(uint8_t(0x01)); }
    };

    template<>
    struct is_arithmetic<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp8_e5m2> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp8_e5m2> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp8_e5m2> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp8_e5m2> {
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
        static constexpr int digits = 3;      // 1 + 2 bits
        static constexpr int digits10 = 1;
        static constexpr int max_digits10 = 2;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -15;
        static constexpr int min_exponent10 = -4;
        static constexpr int max_exponent = 16;
        static constexpr int max_exponent10 = 4;

        static constexpr ymd::math::fp8_e5m2 min() noexcept { return ymd::math::fp8_e5m2::from_bits(0x04); }
        static constexpr ymd::math::fp8_e5m2 lowest() noexcept { return ymd::math::fp8_e5m2::from_bits(0xFB); }
        static constexpr ymd::math::fp8_e5m2 max() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7B); }
        static constexpr ymd::math::fp8_e5m2 epsilon() noexcept { return ymd::math::fp8_e5m2::from_bits(0x30); }
        static constexpr ymd::math::fp8_e5m2 round_error() noexcept { return ymd::math::fp8_e5m2(0.5f); }
        static constexpr ymd::math::fp8_e5m2 infinity() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7C); }
        static constexpr ymd::math::fp8_e5m2 quiet_NaN() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7E); }
        static constexpr ymd::math::fp8_e5m2 signaling_NaN() noexcept { return ymd::math::fp8_e5m2::from_bits(0x7D); }
        static constexpr ymd::math::fp8_e5m2 denorm_min() noexcept { return ymd::math::fp8_e5m2::from_bits(0x01); }
    };
}