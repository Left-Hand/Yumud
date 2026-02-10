#pragma once

#include <cstdint>

// https://zhuanlan.zhihu.com/p/80305502


namespace ymd::str{
__attribute__((always_inline)) [[gnu::const]]
[[nodiscard]] static constexpr uint8_t  div_10( const uint8_t u8_in ) noexcept{
    uint16_t  u16_out = 0xCD;
    u16_out *= u8_in;
    u16_out = u16_out >> 11;
    return static_cast<uint8_t>(u16_out);
}

__attribute__((always_inline)) [[gnu::const]]
[[nodiscard]] static constexpr uint16_t  div_10( const uint16_t u16_in ) noexcept{
    uint32_t  u32_out = 0xCCCD;
    u32_out *= u16_in;
    u32_out = u32_out >> 19;
    return static_cast<uint16_t>(u32_out);
}


__attribute__((always_inline)) [[gnu::const]]
[[nodiscard]] static constexpr uint32_t  div_10( const uint32_t uint_in ) noexcept
{
    // 这里取2的35次方，是因为取到 大于等于 这个值，即算法的推导过程中使用的数字要高于32bit数字8倍以上，
    // 才能够保证任意32bit除数计算的精度，
    // 过去有很多算法论文中给出的 magic number 并不能保证32bit任意数字除十都是完全正确的，是工程中的大坑
    constexpr uint32_t MAGIC = 0xCCCCCCCD;
    static_assert(MAGIC == (1ull << 35) / 10 + 1);
    uint64_t  u64_out = MAGIC;
    u64_out *= uint_in;
    u64_out = u64_out >> 35;
    return  static_cast<uint32_t>(u64_out);
}



__attribute__((always_inline)) [[gnu::const]]

[[nodiscard]] static constexpr uint32_t  div_100( const uint32_t uint_in ) noexcept
{
    constexpr uint32_t MAGIC = 0x51EB851F;
    static_assert(MAGIC == (1ull << 37) / 100 + 1);
    uint64_t  u64_out = MAGIC;
    u64_out *= uint_in;
    u64_out = u64_out >> 37;
    return  static_cast<uint32_t>(u64_out);
}

static_assert(div_100(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 100);

// 核心：除以 10^n 的魔数和移位值表
// 公式：n / 10^k ≈ (n * magic) >> shift


#if 0
// 32位版本（适用于 uint32_t）
struct ReciprocalPow10_32 {
    uint8_t exponent;      // 10的指数 k
    uint32_t divisor;      // 除数 = 10^k
    uint64_t magic;        // 魔数 = ceil(2^shift / 10^k)
    uint8_t shift;         // 移位值
    uint8_t precision_bits;// 精度位数
};

static constexpr ReciprocalPow10_32 reciprocal_pow10_32[] = {
    // k, 除数,       魔数(十六进制),           移位, 精度位
    {0,  1,     0x0000000000000001ULL,     0,  0},    // 除以1
    {1,  10,    0xCCCCCCCCCCCCCCCDULL,     35, 32},   // 除以10 (最常用)
    {2,  100,   0x51EB851EB851EB85ULL,     38, 32},   // 除以100
    {3,  1000,  0x4189374BC6A7EF9DULL,     41, 32},   // 除以1000
    {4,  10000, 0x68DB8BAC710CB295ULL,     45, 32},   // 除以10000
    {5,  100000,0x53E2D6238DA3C211ULL,     48, 32},   // 除以100000
    {6,  1000000,0x431BDE82D7B634DBULL,    52, 32},   // 除以1000000
    {7,  10000000,0x6B5FCA6AF2BD2159ULL,   55, 32},   // 除以10000000
    {8,  100000000,0x55E63B88C230E77EULL,  59, 32},   // 除以100000000
    {9,  1000000000,0x44B82FA09B5A52CBULL, 62, 32},   // 除以1000000000
};
#endif


#if 0
// 64位版本（适用于 uint64_t）
struct ReciprocalPow10_64 {
    uint8_t exponent;      // 10的指数 k
    uint64_t divisor;      // 除数 = 10^k
    __uint128_t magic;     // 128位魔数
    uint8_t shift;         // 移位值
    uint8_t precision_bits;// 精度位数
};

static constexpr ReciprocalPow10_64 reciprocal_pow10_64[] = {
    // k, 除数,             魔数(128位十六进制),                移位, 精度位
    {0,  1ULL,          0x00000000000000000000000000000001ULL, 0,   0},
    {1,  10ULL,         0xCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDULL, 67,  64},
    {2,  100ULL,        0x51EB851EB851EB851EB851EB851EB851ULL, 70,  64},
    {3,  1000ULL,       0x4189374BC6A7EF9DB22D0E5604189375ULL, 73,  64},
    {4,  10000ULL,      0x68DB8BAC710CB295E9E1B089A0275255ULL, 77,  64},
    {5,  100000ULL,     0x53E2D6238DA3C211F0435DADF1A94A5BULL, 80,  64},
    {6,  1000000ULL,    0x431BDE82D7B634DBAA9D5B1E6E37B6D9ULL, 84,  64},
    {7,  10000000ULL,   0x6B5FCA6AF2BD215C7A5C1C2E1B5C1C2FULL, 87,  64},
    {8,  100000000ULL,  0x55E63B88C230E77E5A5C3C3E1A5C3C3FULL, 91,  64},
    {9,  1000000000ULL, 0x44B82FA09B5A52CBAA9D5B1E6E37B6D9ULL, 94,  64},
    {10, 10000000000ULL,0x6F5088CCACD2A6E0A9D5B1E6E37B6D9BULL, 98,  64},
    {11, 100000000000ULL,0x5A5E5A5E5A5E5A5E5A5E5A5E5A5E5A5EULL, 101, 64},
    {12, 1000000000000ULL,0x4A5A5A5A5A5A5A5A5A5A5A5A5A5A5A5AULL, 105, 64},
    {13, 10000000000000ULL,0x76B6B6B6B6B6B6B6B6B6B6B6B6B6B6B7ULL, 108, 64},
    {14, 100000000000000ULL,0x61616161616161616161616161616161ULL, 112, 64},
    {15, 1000000000000000ULL,0x50A0A0A0A0A0A0A0A0A0A0A0A0A0A0A0ULL, 115, 64},
    {16, 10000000000000000ULL,0x820820820820820820820820820821ULL, 119, 64},
    {17, 100000000000000000ULL,0x6B6B6B6B6B6B6B6B6B6B6B6B6B6B6B6BULL, 122, 64},
    {18, 1000000000000000000ULL,0x58D2D2D2D2D2D2D2D2D2D2D2D2D2D2D3ULL, 126, 64},
    {19, 10000000000000000000ULL,0x91A2B3C4D5E6F708192A3B4C5D6E7F8ULL, 129, 64},
#endif
}