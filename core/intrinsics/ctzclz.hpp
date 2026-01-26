#pragma once

#include "bitreverse.hpp"

namespace ymd::intrinsics{
static constexpr bool has_b_clz = __has_builtin(__builtin_clz);
static constexpr bool has_b_ctz = __has_builtin(__builtin_ctz);
static constexpr bool has_b_div = __has_builtin(__builtin_div);
static constexpr bool has_b_sqrt = __has_builtin(__builtin_sqrt);



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

static constexpr uint8_t CLZ_DE_BRUIJN_TABLE[32] = {
    31, 22, 30, 21, 18, 10, 29, 2, 20, 17, 15, 13, 9, 6, 28, 1,
    23, 19, 11, 3, 16, 14, 7, 24, 12, 4, 8, 25, 5, 26, 27, 0
};




static constexpr uint32_t ctz32_nonzero(uint32_t x){
    if(x == 0) __builtin_unreachable();
    // 获取最低有效位
    uint32_t lsb = x & -x;
    // 使用德布鲁因常数进行哈希
    uint32_t index = (lsb * 0x077CB531U) >> 27;
    return CTZ_DE_BRUIJN_TABLE[index];
}
static constexpr uint32_t ctz32(uint32_t x) {
    if (x == 0) return 32;
    return ctz32_nonzero(x);
}

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
