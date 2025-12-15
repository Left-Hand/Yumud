#pragma once

#include <cstdint>
#include <bit>
#include <algorithm>

// Apache-2.0 license
// https://github.com/tarcieri/micromath/blob/main/src/float/atan2.rs

namespace ymd::fastmath{

/// Approximates `atan2(y,x)` normalized to the `[0, 4)` range with a maximum
/// error of `0.1620` degrees.
static constexpr float atan2_norm(float y, float x){
    constexpr uint32_t SIGN_MASK = 0x8000'0000;
    constexpr float B = 0.596'227;


    // Extract sign bits from floating point values
    const uint32_t ux_s = SIGN_MASK & std::bit_cast<uint32_t>(x);
    const uint32_t uy_s = SIGN_MASK & std::bit_cast<uint32_t>(y);

    // Determine quadrant offset
    const auto q = float((!ux_s & uy_s) >> 29 | ux_s >> 30);

    // Calculate arctangent in the first quadrant
    const float bxy_a = std::abs(B * x * y);
    const float n = bxy_a + y * y;
    const float atan_1q = n / (x * x + bxy_a + n);

    // Translate it to the proper quadrant
    const uint32_t uatan_2q = (ux_s ^ uy_s) | std::bit_cast<uint32_t>(atan_1q);
    return float(q) + std::bit_cast<float>(uatan_2q);
}


static constexpr float atan_norm(float x){
    constexpr uint32_t SIGN_MASK = 0x8000'0000;
    constexpr float B = 0.596'227;

    // Extract the sign bit
    const uint32_t ux_s = SIGN_MASK & std::bit_cast<uint32_t>(x);

    // Calculate the arctangent in the first quadrant
    const float bx_a = std::abs(B * x);
    const float n = bx_a + x * x;
    const float atan_1q = n / (1.0 + bx_a + n);

    // Restore the sign bit and convert to float
    return std::bit_cast<float>(ux_s | std::bit_cast<uint32_t>(atan_1q));
}
}