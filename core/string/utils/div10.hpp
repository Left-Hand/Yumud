#pragma once

#include <cstdint>

// https://zhuanlan.zhihu.com/p/80305502


namespace ymd::str{
__attribute__((always_inline))
[[nodiscard]] static constexpr uint8_t  div_10( const uint8_t u8_in ) noexcept{
    uint16_t  u16_out = 0xCD;
    u16_out *= u8_in;
    u16_out = u16_out >> 11;
    return static_cast<uint8_t>(u16_out);
}

__attribute__((always_inline))
[[nodiscard]] static constexpr uint16_t  div_10( const uint16_t u16_in ) noexcept{
    uint32_t  u32_out = 0xCCCD;
    u32_out *= u16_in;
    u32_out = u32_out >> 19;
    return static_cast<uint16_t>(u32_out);
}


__attribute__((always_inline))
[[nodiscard]] static constexpr uint32_t  div_10( const uint32_t uint_in ) noexcept
{
    // 这里取2的35次方，是因为取到 大于等于 这个值，即算法的推导过程中使用的数字要高于32bit数字8倍以上，
    // 才能够保证任意32bit除数计算的精度，
    // 过去有很多算法论文中给出的 magic number 并不能保证32bit任意数字除十都是完全正确的，是工程中的大坑
    uint64_t  u64_out = 0xCCCCCCCD;
    u64_out *= uint_in;
    u64_out = u64_out >> 35;
    return  static_cast<uint32_t>(u64_out);
}

static_assert(div_10(uint32_t(0xFFFFFFFF)) == 0xFFFFFFFF / 10);



}