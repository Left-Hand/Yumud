#pragma once

#include "core/utils/bytes/bytes_caster.hpp"

namespace ymd::intrinsics{
// 方法3：手动组合两个对齐的字（如你要求的）
[[nodiscard]] static constexpr uint32_t load_u32_may_unaligned(const uint8_t *addr) {
    if(std::is_constant_evaluated()){
        return le_bytes_to_int<uint32_t>(std::span<const uint8_t, 4>(addr, 4));
    }
    const uintptr_t addr_val = reinterpret_cast<uintptr_t>(addr);
    const uint32_t *aligned_addr = reinterpret_cast<const uint32_t*>(addr_val & ~3);

    const uint32_t low_word = aligned_addr[0];
    const uint32_t high_word = aligned_addr[1];
    const unsigned int shift = (addr_val & 3);

    // 预计算的掩码表
    [[maybe_unused]] constexpr uint32_t RIGHT_SHIFT[4] = {0, 8, 16, 24};
    [[maybe_unused]] constexpr uint32_t LEFT_SHIFT[4] = {0, 24, 16, 8};

    switch(shift){
        case 0: return low_word;
        case 1: return (low_word >> 8) | (high_word << 24);
        case 2: return (low_word >> 16) | (high_word << 16);
        case 3: return (low_word >> 24) | (high_word << 8);
        default: __builtin_unreachable();
    }
    // return (low_word >> RIGHT_SHIFT[shift]) | (high_word << LEFT_SHIFT[shift]);
}
}
