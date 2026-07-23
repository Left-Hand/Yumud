#pragma once

#include "core/math/realmath.hpp"

namespace ymd::math{
static constexpr float ln(const float x){
    return std::logf(x);
}
}

static constexpr float T0_KELVIN = 273.15f + 25.0f;  // 298.15K
static constexpr float KELVIN_OFFSET = 273.15f;
static constexpr float INV_T0_KELVIN = 1.0f / T0_KELVIN;


namespace ymd::drivers{
// 使用 B 参数公式实现 NTC 温度计算
struct [[nodiscard]] NtcCalculator final {


    uq32 inv_b0;          // 1 / B0 (B常数倒数)
    uq32 inv_r0_kohms;    // 1 / R0 (25°C时电阻倒数)

    // 工厂方法：从 B 值和 R0 创建
    static constexpr NtcCalculator from_b0r0(const uint32_t b0, const uq16 r0_kohms) {
        return {
            .inv_b0 = uq32::from_rcp(b0),
            .inv_r0_kohms = uq32(1 / r0_kohms)
        };
    }

    // NTC电阻值 → 温度 (°C)
    [[nodiscard]] constexpr iq16 kohms_to_celsius(const uq16 rt_kohms) const {
        const iq16 inv_kelvin = iq16(INV_T0_KELVIN) + math::ln(rt_kohms * inv_r0_kohms) * inv_b0;
        return (1 / inv_kelvin) - iq16(KELVIN_OFFSET);
    }
};


// 使用 B 参数公式实现 NTC 温度计算
struct [[nodiscard]] NtcCalculatorF final {


    float inv_b0;          // 1 / B0 (B常数倒数)
    float inv_r0_kohms;    // 1 / R0 (25°C时电阻倒数)

    // 工厂方法：从 B 值和 R0 创建
    static constexpr NtcCalculatorF from_b0r0(const uint32_t b0, const float r0_kohms) {
        return {
            .inv_b0 = 1.0f / static_cast<float>(b0),
            .inv_r0_kohms = 1.0f / r0_kohms
        };
    }

    // NTC电阻值 → 温度 (°C)
    [[nodiscard]] constexpr float kohms_to_celsius(const float rt_kohms) const {
        const float inv_kelvin = INV_T0_KELVIN + math::ln(rt_kohms * inv_r0_kohms) * inv_b0;
        return (1.0f / inv_kelvin) - KELVIN_OFFSET;
    }
};


};