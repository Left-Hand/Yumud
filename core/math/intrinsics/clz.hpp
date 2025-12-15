#pragma once

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


// static constexpr uint32_t ctz_fast_mul(uint32_t x) {
//     if (x == 0) return 32;
//     uint32_t lsb = x & -x;
//     // 德布鲁因常数
//     uint32_t index = (lsb * 0x077CB531U) >> 27;
//     // 非常小的表（32字节）
//     constexpr uint8_t table[32] = {
//         0, 1, 2, 6, 3, 11, 7, 16, 4, 14, 12, 21, 8, 23, 17, 26,
//         31, 5, 10, 15, 13, 20, 22, 25, 30, 9, 19, 24, 29, 18, 28, 27
//     };
//     return table[index];
// }

// 正确的德布鲁因表用于ctz
static constexpr uint8_t ctz_de_bruijn_table[32] = {
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

static constexpr uint8_t clz_de_bruijn_table[32] = {
    31, 22, 30, 21, 18, 10, 29, 2, 20, 17, 15, 13, 9, 6, 28, 1,
    23, 19, 11, 3, 16, 14, 7, 24, 12, 4, 8, 25, 5, 26, 27, 0
};

static constexpr uint32_t ctz_fast_mul(uint32_t x) {
    if (x == 0) return 32;
    // 获取最低有效位
    uint32_t lsb = x & -x;
    // 使用德布鲁因常数进行哈希
    uint32_t index = (lsb * 0x077CB531U) >> 27;
    return ctz_de_bruijn_table[index];
}

// 如果没有__builtin_bitreverse32，可以手动实现位反转
static constexpr uint32_t bitreverse32(uint32_t x) {
    x = ((x & 0xAAAAAAAA) >> 1) | ((x & 0x55555555) << 1);
    x = ((x & 0xCCCCCCCC) >> 2) | ((x & 0x33333333) << 2);
    x = ((x & 0xF0F0F0F0) >> 4) | ((x & 0x0F0F0F0F) << 4);
    x = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
    return (x >> 16) | (x << 16);
}

__attribute__((noinline)) static constexpr uint32_t my_clz(uint32_t x) {
    if (x == 0) return 32;
    
    // 反转位顺序
    // 通过位反转，clz就变成了ctz
    x = bitreverse32(x);  // GCC/Clang内置函数
    
    // 现在使用ctz的方法
    uint32_t lsb = x & -x;
    uint32_t index = (lsb * 0x077CB531U) >> 27;

    
    return ctz_de_bruijn_table[index];
}

static_assert(ctz_fast_mul(0) == 32, "ctz(0) should be 32");
static_assert(ctz_fast_mul(1) == 0, "ctz(1) should be 0");
static_assert(ctz_fast_mul(2) == 1, "ctz(2) should be 1");
static_assert(ctz_fast_mul(3) == 0, "ctz(3) should be 0");
static_assert(ctz_fast_mul(4) == 2, "ctz(4) should be 2");
static_assert(ctz_fast_mul(5) == 0, "ctz(5) should be 0");
static_assert(ctz_fast_mul(6) == 1, "ctz(6) should be 1");
static_assert(ctz_fast_mul(7) == 0, "ctz(7) should be 0");
static_assert(ctz_fast_mul(8) == 3, "ctz(8) should be 3");
static_assert(ctz_fast_mul(0x80000000) == 31, "ctz(0x80000000) should be 31");
static_assert(ctz_fast_mul(0xFFFFFFFF) == 0, "ctz(0xFFFFFFFF) should be 0");
static_assert(ctz_fast_mul(0x00010000) == 16, "ctz(0x00010000) should be 16");
static_assert(ctz_fast_mul(0x00008000) == 15, "ctz(0x00008000) should be 15");
static_assert(ctz_fast_mul(0x00000010) == 4, "ctz(0x00000010) should be 4");
static_assert(ctz_fast_mul(0x00000020) == 5, "ctz(0x00000020) should be 5");
static_assert(ctz_fast_mul(0x00000400) == 10, "ctz(0x00000400) should be 10");
static_assert(ctz_fast_mul(0x00000800) == 11, "ctz(0x00000800) should be 11");
static_assert(ctz_fast_mul(0x00001000) == 12, "ctz(0x00001000) should be 12");
static_assert(ctz_fast_mul(0x00002000) == 13, "ctz(0x00002000) should be 13");
static_assert(ctz_fast_mul(0x00004000) == 14, "ctz(0x00004000) should be 14");
static_assert(ctz_fast_mul(0x00008000) == 15, "ctz(0x00008000) should be 15");
static_assert(ctz_fast_mul(0x00010000) == 16, "ctz(0x00010000) should be 16");
static_assert(ctz_fast_mul(0x00020000) == 17, "ctz(0x00020000) should be 17");
static_assert(ctz_fast_mul(0x00040000) == 18, "ctz(0x00040000) should be 18");
static_assert(ctz_fast_mul(0x00080000) == 19, "ctz(0x00080000) should be 19");
static_assert(ctz_fast_mul(0x00100000) == 20, "ctz(0x00100000) should be 20");
static_assert(ctz_fast_mul(0x00200000) == 21, "ctz(0x00200000) should be 21");
static_assert(ctz_fast_mul(0x00400000) == 22, "ctz(0x00400000) should be 22");
static_assert(ctz_fast_mul(0x00800000) == 23, "ctz(0x00800000) should be 23");
static_assert(ctz_fast_mul(0x01000000) == 24, "ctz(0x01000000) should be 24");
static_assert(ctz_fast_mul(0x02000000) == 25, "ctz(0x02000000) should be 25");
static_assert(ctz_fast_mul(0x04000000) == 26, "ctz(0x04000000) should be 26");
static_assert(ctz_fast_mul(0x08000000) == 27, "ctz(0x08000000) should be 27");
static_assert(ctz_fast_mul(0x10000000) == 28, "ctz(0x10000000) should be 28");
static_assert(ctz_fast_mul(0x20000000) == 29, "ctz(0x20000000) should be 29");
static_assert(ctz_fast_mul(0x40000000) == 30, "ctz(0x40000000) should be 30");
static_assert(ctz_fast_mul(0x80000000) == 31, "ctz(0x80000000) should be 31");

// 测试
static_assert(my_clz(0) == 32, "clz(0) should be 32");
static_assert(my_clz(1) == 31, "clz(1) should be 31");
static_assert(my_clz(2) == 30, "clz(2) should be 30");
static_assert(my_clz(3) == 30, "clz(3) should be 30");
static_assert(my_clz(4) == 29, "clz(4) should be 29");
static_assert(my_clz(0x80000000) == 0, "clz(0x80000000) should be 0");
static_assert(my_clz(0x40000000) == 1, "clz(0x40000000) should be 1");
static_assert(my_clz(0x00008000) == 16, "clz(0x00008000) should be 16");
static_assert(my_clz(0x00010000) == 15, "clz(0x00010000) should be 15");
static_assert(my_clz(0xFFFFFFFF) == 0, "clz(0xFFFFFFFF) should be 0");