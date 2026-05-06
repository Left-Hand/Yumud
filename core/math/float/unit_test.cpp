#include "bf16.hpp"
#include "fp16.hpp"
#include "fp32.hpp"
#include "fp24.hpp"

using namespace ymd;
using namespace math;


namespace {
namespace test_fp16 {
    // 整数转换测试
    constexpr auto zero = fp16(0);
    static_assert(zero.to_bits() == 0x0000);

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

    // constexpr auto quarter = fp16(0.25f);
    // static_assert(quarter.to_bits() == 0x3000);

    // 反向转换测试
    static_assert(static_cast<float>(fp16(1)) == 1.0f);
    static_assert(static_cast<float>(fp16(2)) == 2.0f);
    static_assert(static_cast<float>(fp16(-1)) == -1.0f);
    static_assert(static_cast<float>(fp16(0.5f)) == 0.5f);
}


// 大数值和边界测试
static_assert(fp16(65504.0f).to_bits() == 0x7BFF); // 最大有限值
static_assert(fp16(65504).to_bits() == 0x7BFF);    // 整数最大值

// // 小数值测试
// static_assert(fp16(0.0009765625f).to_bits() == 0x0400); // 最小规范数

// NaN 检测测试
static_assert(fp16(std::numeric_limits<float>::quiet_NaN()).is_nan());


}