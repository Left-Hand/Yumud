#include "../fp16.hpp"
#include <cmath>

using namespace ymd;
using namespace math;


namespace {

[[maybe_unused]] static void test_fp16(){
    {

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
        static_assert(min_normal.to_bits() == 0x03ff);
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

    {
        static_assert(fp16::from_bits(0x0000U).to_bits() == 0x0000U);
        static_assert(fp16::from_bits(0x8000U).to_bits() == 0x8000U);
        static_assert(math::isinf(fp16::from_bits(0x7C00U)));
        static_assert(math::isinf(fp16::from_bits(0xFC00U)));

        static_assert(fp16::from_bits(0x7E00U).is_nan());
        static_assert(!fp16::from_bits(0x7BFFU).is_nan());

        static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0x0000U).to_f32()) == 0x00000000U);
        // static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0x8000U).to_f32()) == 0x80000000U);
        static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0x3C00U).to_f32()) == 0x3F800000U); // 1.0
        static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0xC000U).to_f32()) == 0xC0000000U); // -2.0
        static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0x0400U).to_f32()) == 0x38800000U); // min normal
        // static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0x0001U).to_f32()) == 0x33800000U); // min subnormal
        // static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0x7C00U).to_f32()) == 0x7F800000U); // +inf
        // static_assert(std::bit_cast<uint32_t>(fp16::from_bits(0xFC00U).to_f32()) == 0xFF800000U); // -inf

        static_assert(fp16(0.0f).to_bits() == 0x0000U);
        static_assert(fp16(-0.0f).to_bits() == 0x8000U);
        static_assert(fp16(1.0f).to_bits() == 0x3C00U);
        static_assert(fp16(-2.0f).to_bits() == 0xC000U);
        static_assert(fp16(65504.0f).to_bits() == 0x7BFFU);
        static_assert(fp16(0x1p-24f).to_bits() == 0x0001U);
        // static_assert(fp16(0x1p-25f).to_bits() == 0x0000U);
        static_assert(fp16(0x1.8p-24f).to_bits() == 0x0002U);
        // static_assert(fp16(1.00048828125f).to_bits() == 0x3C00U);
        static_assert(fp16(1.00146484375f).to_bits() == 0x3C02U);
        static_assert(fp16(1.0009765625f).to_bits() == 0x3C01U);
        static_assert(fp16(0x1p16f).to_bits() == 0x7C00U);

        static_assert(std::is_convertible_v<float, fp16>);
        static_assert(!std::is_convertible_v<fp16, float>);
        static_assert([]{
            const float value = 1.0f;
            const fp16 half = value;
            return half.to_bits() == 0x3C00U;
        }());
        static_assert([]{
            fp16 half;
            half = 2.0f;
            return half.to_bits() == 0x4000U;
        }());

    }
}


}