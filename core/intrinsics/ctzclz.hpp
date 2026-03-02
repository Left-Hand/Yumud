#pragma once

#include "bitreverse.hpp"

namespace ymd::intrinsics{
static constexpr bool has_b_clz = __has_builtin(__builtin_clz);
static constexpr bool has_b_ctz = __has_builtin(__builtin_ctz);
static constexpr bool has_b_div = __has_builtin(__builtin_div);
static constexpr bool has_b_sqrt = __has_builtin(__builtin_sqrt);

#if 0
[[nodiscard]] static constexpr uint32_t __ymd_ctz_impl(uint32_t x) {
    // under both the University of Illinois "BSD-Like" license and the MIT license
    //https://github.com/microsoft/compiler-rt/blob/master/lib/builtins/ctzsi2.c

    int32_t t = ((x & 0x0000FFFF) == 0) << 4;  /* if (x has no small bits) t = 16 else 0 */
    x >>= t;           /* x = [0 - 0xFFFF] + higher garbage bits */
    uint32_t r = t;       /* r = [0, 16]  */
    /* return r + ctz(x) */
    t = ((x & 0x00FF) == 0) << 3;
    x >>= t;           /* x = [0 - 0xFF] + higher garbage bits */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + ctz(x) */
    t = ((x & 0x0F) == 0) << 2;
    x >>= t;           /* x = [0 - 0xF] + higher garbage bits */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + ctz(x) */
    t = ((x & 0x3) == 0) << 1;
    x >>= t;
    x &= 3;            /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */

    return r + ((2 - (x >> 1)) & -((x & 1) == 0));
}

[[nodiscard]] static constexpr uint32_t  __ymd_clz_impl(uint32_t x){
    // under both the University of Illinois "BSD-Like" license and the MIT license
    // https://github.com/m-labs/compiler-rt-lm32/blob/master/lib/clzsi2.c

    int32_t t = ((x & 0xFFFF0000) == 0) << 4;  /* if (x is small) t = 16 else 0 */
    x >>= 16 - t;      /* x = [0 - 0xFFFF] */
    uint32_t r = t;       /* r = [0, 16] */
    /* return r + clz(x) */
    t = ((x & 0xFF00) == 0) << 3;
    x >>= 8 - t;       /* x = [0 - 0xFF] */
    r += t;            /* r = [0, 8, 16, 24] */
    /* return r + clz(x) */
    t = ((x & 0xF0) == 0) << 2;
    x >>= 4 - t;       /* x = [0 - 0xF] */
    r += t;            /* r = [0, 4, 8, 12, 16, 20, 24, 28] */
    /* return r + clz(x) */
    t = ((x & 0xC) == 0) << 1;
    x >>= 2 - t;       /* x = [0 - 3] */
    r += t;            /* r = [0 - 30] and is even */

    return r + ((2 - x) & -((x & 2) == 0));
}
#endif

#ifdef __riscv_zbb
// 如果有Zbb扩展，直接用硬件指令
static inline uint32_t ctz_riscv(uint32_t x) {
    uint32_t result;
    __asm__ volatile ("ctz %0, %1" : "=r"(result) : "r"(x));
    return result;
}
static constexpr bool riscv_has_native_ctz = true;
#else
static constexpr bool riscv_has_native_ctz = false;
#endif

// 正确的德布鲁因表用于ctz
static constexpr uint8_t CTZ_DE_BRUIJN_TABLE[32] = {
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

static constexpr uint32_t ctz32_nonzero(uint32_t x){
    if(x == 0) __builtin_unreachable();
    // 获取最低有效位
    uint32_t temp = x & -x;
    // 使用德布鲁因常数进行哈希
    return CTZ_DE_BRUIJN_TABLE[uint32_t(temp * 0x077CB531U) >> 27];
}


static constexpr uint32_t ctz32(uint32_t x) {
    if (x == 0) return 32;
    return ctz32_nonzero(x);
}

static constexpr uint8_t CLZ_DE_BRUIJN_TABLE[32] = {
    31, 22, 30, 21, 18, 10, 29, 2, 20, 17, 15, 13, 9, 6, 28, 1,
    23, 19, 11, 3, 16, 14, 7, 24, 12, 4, 8, 25, 5, 26, 27, 0
};

[[nodiscard]] static constexpr uint32_t clz32_nonzero(uint32_t x) {
    if(x == 0) __builtin_unreachable();
    // 反转位顺序
    // 通过位反转，clz就变成了ctz
    x = bitreverse32(x);  // GCC/Clang内置函数
    return ctz32_nonzero(x);
}


[[nodiscard]] static constexpr uint32_t clz32(uint32_t x) {
    if (x == 0) return 32;
    return clz32_nonzero(x);
}


}
