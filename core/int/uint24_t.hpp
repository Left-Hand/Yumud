#pragma once

#include "stdint.h"
#include "core/platform.hpp"


namespace ymd::math{
struct uint24_t{
    uint32_t bits : 24;

    __fast_inline constexpr uint24_t();

    __fast_inline constexpr explicit uint24_t(const uint24_t & other) {
        bits = other.bits;
    }

    __fast_inline constexpr explicit uint24_t(const uint32_t other) {
        bits = other & 0xFFFFFF;
    }

    __fast_inline constexpr explicit uint24_t(const int other) {
        bits = other & 0xFFFFFF;
    }

    __fast_inline constexpr explicit operator uint32_t() const {
        return bits;
    }

};

struct int24_t{
    int32_t bits : 24;

    __fast_inline constexpr int24_t();

    __fast_inline constexpr explicit int24_t(const int24_t & other) {
        bits = other.bits;
    }

    __fast_inline constexpr explicit int24_t(const int32_t other) {
        bits = other & 0xFFFFFF;
    }

    __fast_inline constexpr explicit int24_t(const int other) {
        bits = other & 0xFFFFFF;
    }

    __fast_inline constexpr explicit operator int32_t() const {
        return bits;
    }

};

}

