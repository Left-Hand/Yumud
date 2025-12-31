#pragma once

#include <span>
#include <cstdint>

namespace ymd::robots::bmkj::m1502e{
struct [[nodiscard]] BytesFiller final{
    constexpr explicit BytesFiller(std::span<uint8_t> bytes): bytes_(bytes){}

    constexpr void push_be_u16(uint16_t value){ 
        if(offset_ + 2 > bytes_.size())
            __builtin_trap();
        bytes_[offset_++] = value >> 8;
        bytes_[offset_++] = value;
    }

    constexpr void push_u8(const uint8_t value){
        if(offset_ + 1 > bytes_.size())
            __builtin_trap();
        bytes_[offset_++] = value;
    }

    constexpr void push_repeat_u8(const uint8_t value, size_t count){ 
        if(offset_ + count > bytes_.size())
            __builtin_trap();
        for(size_t i = 0; i < count; i++)
            bytes_[offset_++] = value;

    }
private:
    std::span<uint8_t> bytes_;
    size_t offset_ = 0;
};
}