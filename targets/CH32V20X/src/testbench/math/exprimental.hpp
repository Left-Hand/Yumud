#include "core/math/fixed/fxmath.hpp"
#include "core/math/fixed/fixed.hpp"
#include <cmath>


namespace exprimental{



static constexpr ymd::math::fixed<32, uint32_t> iq31_length_squared(const ymd::math::fixed<31, int32_t> x){
    const auto abs_x_bits = x.to_bits() < 0 ? static_cast<uint32_t>(-x.to_bits()) : static_cast<uint32_t>(x.to_bits());
    const auto bits = static_cast<uint64_t>(abs_x_bits) * static_cast<uint64_t>(abs_x_bits);
    return ymd::math::fixed<32, uint32_t>::from_bits(static_cast<uint32_t>(bits >> 30));
}
static_assert(iq31_length_squared(ymd::math::fixed<31, int32_t>(0.5)) == ymd::iq32(0.25));

static constexpr ymd::math::fixed<32, uint32_t> dual_iq31_length_squared(const ymd::math::fixed<31, int32_t> x, const ymd::math::fixed<31, int32_t> y){
    // IQ31 格式：1 位符号 + 31 位小数
    
    // 方法1：使用数学运算（推荐）
    int32_t x_bits = x.to_bits();
    int32_t y_bits = y.to_bits();
    
    // 转换为 int64_t 进行计算，避免溢出
    int64_t x_val = static_cast<int64_t>(x_bits);  // 实际上是 Q31
    int64_t y_val = static_cast<int64_t>(y_bits);
    
    // 计算平方：Q31 × Q31 = Q62
    int64_t x_sq = (x_val * x_val);  // Q62
    int64_t y_sq = (y_val * y_val);  // Q62
    
    // 相加：Q62 + Q62 = Q62
    int64_t sum = x_sq + y_sq;  // Q62
    
    // 转换为 UQ32：从 Q62 到 Q32，需要右移 30 位
    // 但注意：sum 可能为负数（虽然理论上不会）
    if (sum < 0) {
        // 理论上不应该发生，但安全处理
        sum = 0;
    }
    
    // 右移 30 位得到 Q32（62-30=32）
    // 同时进行饱和处理
    constexpr int64_t max_uq32 = static_cast<int64_t>(std::numeric_limits<uint32_t>::max());
    int64_t shifted = sum >> 30;
    
    if (shifted > max_uq32) {
        shifted = max_uq32;
    }
    
    return ymd::math::fixed<32, uint32_t>::from_bits(static_cast<uint32_t>(shifted));
}

static_assert(dual_iq31_length_squared(ymd::iq31(0.5), ymd::iq31(0.5)).to_bits() == (ymd::uq32(0.5)).to_bits());



}


