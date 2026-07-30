
#include "../fp8.hpp"

using namespace ymd;
using namespace math;


namespace {


[[maybe_unused]] static void test_fp8() {
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