#pragma once

#include <cstdint>

namespace ymd::intrinsics{

// 如果没有__builtin_bitreverse32，可以手动实现位反转
static constexpr uint32_t bitreverse32(uint32_t x) {
    #if 0
    constexpr bool has_builtin = __has_builtin(__builtin_bitreverse32);
    if constexpr (has_builtin){
        return __builtin_bitreverse32(x);
    }
    #endif
    x = ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
    x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
    x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
    x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
    return (x >> 16) | (x << 16);
}

static constexpr uint16_t bitreverse16(uint16_t x) {
    #if 0
    constexpr bool has_builtin = __has_builtin(__builtin_bitreverse16);
    if constexpr (has_builtin){
        return __builtin_bitreverse16(x);
    }
    #endif
    x = ((x & 0xAAAA) >> 1) | ((x & 0x5555) << 1);
    x = ((x & 0xCCCC) >> 2) | ((x & 0x3333) << 2);
    return (x >> 8) | (x << 8);
}

static constexpr uint8_t bitreverse8(uint8_t x) {
    x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
    return (x >> 4) | (x << 4);
}
}
