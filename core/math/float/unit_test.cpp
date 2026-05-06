#include "bf16.hpp"
#include "fp16.hpp"
#include "fp32.hpp"
#include "fp24.hpp"
#include "fp64.hpp"

#include "fp8.hpp"

using namespace ymd;
using namespace math;


namespace {
namespace test_fp16 {
    // ===== FP16 基础测试 =====
    // 对应形式化验证：core/math/float/formal_verification/verify_fp16_*.py

    // 零值测试
    constexpr auto zero = fp16(0);
    static_assert(zero.to_bits() == 0x0000);

    constexpr auto neg_zero = fp16(-0.0f);
    static_assert(neg_zero.to_bits() == 0x8000);

    // 整数转换测试
    constexpr auto one = fp16(1);
    static_assert(one.to_bits() == 0x3C00);

    constexpr auto neg_one = fp16(-1);
    static_assert(neg_one.to_bits() == 0xBC00);

    constexpr auto two = fp16(2);
    static_assert(two.to_bits() == 0x4000);

    constexpr auto four = fp16(4);
    static_assert(four.to_bits() == 0x4400);

    constexpr auto neg_two = fp16(-2);
    static_assert(neg_two.to_bits() == 0xC000);

    // 浮点转换测试
    constexpr auto half = fp16(0.5f);
    static_assert(half.to_bits() == 0x3800);

    constexpr auto quarter = fp16(0.25f);
    static_assert(quarter.to_bits() == 0x3400);

    constexpr auto eight = fp16(8.0f);
    static_assert(eight.to_bits() == 0x4800);

    // 边界值测试
    constexpr auto min_normal = fp16(0.000061f);  // ≈ 2^-14（最小规范数）
    constexpr auto max_normal = fp16(65504.0f);   // 最大规范值
    static_assert(max_normal.to_bits() == 0x7BFF);

    // 符号位保留性测试（往返验证）
    static_assert((fp16(5.0f).to_bits() & 0x8000) == 0x0000);   // 正数，符号=0
    static_assert((fp16(-5.0f).to_bits() & 0x8000) == 0x8000);  // 负数，符号=1

    // 反向转换测试（FP16 → FP32）
    static_assert(static_cast<float>(fp16(1)) == 1.0f);
    static_assert(static_cast<float>(fp16(2)) == 2.0f);
    static_assert(static_cast<float>(fp16(-1)) == -1.0f);
    static_assert(static_cast<float>(fp16(0.5f)) == 0.5f);
    static_assert(static_cast<float>(fp16(4.0f)) == 4.0f);
    static_assert(static_cast<float>(fp16(8.0f)) == 8.0f);

    // 往返转换一致性（双方向验证）
    // FP32 → FP16 → FP32 应该恢复原值（在 FP16 精度范围内）
    constexpr auto rt1 = static_cast<float>(fp16(1.5f));
    static_assert(rt1 == 1.5f);

    constexpr auto rt2 = static_cast<float>(fp16(2.5f));
    static_assert(rt2 == 2.5f);

    // 特殊值测试
    constexpr auto pos_inf = fp16(std::numeric_limits<float>::infinity());
    static_assert(pos_inf.to_bits() == 0x7C00);

    constexpr auto neg_inf = fp16(-std::numeric_limits<float>::infinity());
    static_assert(neg_inf.to_bits() == 0xFC00);

    // NaN 检测测试
    static_assert(fp16(std::numeric_limits<float>::quiet_NaN()).is_nan());
}



namespace test_fp8 {
    // ===== FP8-E4M3 基础测试 =====
    // 对应形式化验证：core/math/float/formal_verification/verify_fp8_e4m3.py

    // 零值测试
    constexpr auto zero_e4m3 = fp8_e4m3(0);
    static_assert(zero_e4m3.to_bits() == 0x00);

    // 单位元素测试
    constexpr auto one_e4m3 = fp8_e4m3(1.0f);
    static_assert(one_e4m3.to_bits() == 0x38);

    constexpr auto neg_one_e4m3 = fp8_e4m3(-1.0f);
    static_assert(neg_one_e4m3.to_bits() == 0xB8);

    // 幂次测试
    constexpr auto eight_e4m3 = fp8_e4m3(8.0f);  // 2^3, exp=3 → 0x50
    static_assert(eight_e4m3.to_bits() == 0x50);

    constexpr auto two_e4m3 = fp8_e4m3(2.0f);    // 2^1, exp=1 → 0x40
    static_assert(two_e4m3.to_bits() == 0x40);

    constexpr auto half_e4m3 = fp8_e4m3(0.5f);   // 2^-1, exp=-1 → 0x30
    static_assert(half_e4m3.to_bits() == 0x30);

    // 边界值测试：最小规范数 (2^-6)
    constexpr auto min_norm_e4m3 = fp8_e4m3(0.015625f);  // 2^-6, exp=-6 → 0x08
    static_assert(min_norm_e4m3.to_bits() == 0x08);

    constexpr auto neg_min_norm_e4m3 = fp8_e4m3(-0.015625f);  // -2^-6 → 0x88
    static_assert(neg_min_norm_e4m3.to_bits() == 0x88);

    // 最大规范值 (2^7 * 1.875)
    constexpr auto max_norm_e4m3 = fp8_e4m3(240.0f);  // exp=7, mant=7 → 0x77
    static_assert(max_norm_e4m3.to_bits() == 0x77);

    // 溢出测试：exp > 7
    constexpr auto overflow_e4m3 = fp8_e4m3(1e6f);
    static_assert(overflow_e4m3.to_bits() == 0x7E);

    // 下溢测试：exp < -8
    constexpr auto underflow_e4m3 = fp8_e4m3(1e-8f);
    static_assert(underflow_e4m3.to_bits() == 0x00);

    // 符号位保留性测试
    static_assert((fp8_e4m3(5.0f).to_bits() & 0x80) == 0x00);   // 正数
    static_assert((fp8_e4m3(-5.0f).to_bits() & 0x80) == 0x80);  // 负数

    // 反向转换一致性测试（FP8 → FP32）
    // 关键：往返转换后的值应该相等（在 FP8 精度范围内）
    static_assert(static_cast<float>(fp8_e4m3(1.0f)) == 1.0f);
    static_assert(static_cast<float>(fp8_e4m3(0.5f)) == 0.5f);
    static_assert(static_cast<float>(fp8_e4m3(2.0f)) == 2.0f);
    static_assert(static_cast<float>(fp8_e4m3(8.0f)) == 8.0f);
    static_assert(static_cast<float>(fp8_e4m3(0.015625f)) == 0.015625f);  // 2^-6

    // 往返转换验证（双方向一致性）
    // 注意：FP8 → FP32 精度较低，某些转换可能丢失信息
    constexpr auto forward_backward_1 = static_cast<float>(fp8_e4m3(1.0f));
    static_assert(forward_backward_1 == 1.0f);

    constexpr auto forward_backward_half = static_cast<float>(fp8_e4m3(0.5f));
    static_assert(forward_backward_half == 0.5f);

    // ===== FP8-E5M2 基础测试 =====
    constexpr auto zero_e5m2 = fp8_e5m2(0);
    static_assert(zero_e5m2.to_bits() == 0x00);

    // 反向转换一致性测试
    static_assert(static_cast<float>(fp8_e5m2(1.0f)) == 1.0f);
}

}