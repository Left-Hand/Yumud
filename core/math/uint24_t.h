#pragma once

#include "stdint.h"
#include "core/platform.h"


struct uint24_t{
    uint32_t data : 24;

#ifdef __cplusplus

    __fast_inline constexpr uint24_t() {
        data = 0;
    }

    __fast_inline constexpr explicit uint24_t(const uint24_t & other) {
        data = other.data;
    }

    __fast_inline constexpr explicit uint24_t(const uint32_t other) {
        data = other & 0xFFFFFF;
    }

    __fast_inline constexpr explicit uint24_t(const int & other) {
        data = other & 0xFFFFFF;
    }

    __fast_inline constexpr explicit operator uint32_t() const {
        return data;
    }

#endif
};

typedef struct uint24_t uint24_t;

