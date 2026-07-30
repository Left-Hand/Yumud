#pragma once

#include <cstdint>
#include <bit>

namespace ymd::math::fp8_utils{

// FP8-E4M3 格式：1 符号位 | 4 指数位(偏差=7) | 3 尾数位
// 范围：[-240, 240]，最小规范数：2^-6 ≈ 0.015625
// 形式化验证：见 core/math/float/formal_verification/verify_fp8_e4m3.py
// 所有约束都已通过 Z3 验证，包括：
//   - 符号位保留性
//   - 指数溢出/下溢处理
//   - 指数范围 [-8, 7] 映射到 [0, 15]
//   - 单调性保证
__attribute__((optimize("-Ofast"), const))
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
__attribute__((optimize("-Ofast"), const))
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

__attribute__((optimize("-Ofast"), const))
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

__attribute__((optimize("-Ofast"), const))
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


