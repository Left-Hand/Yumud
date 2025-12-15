#pragma once

#include <cstdint>
#include <array>
#include <span>
#include <bit>

// 描述了一个Ipv6地址

namespace ymd {
    class OutputStream;
}

namespace ymd {
struct [[nodiscard]] Ipv6 {
    static_assert(std::endian::native == std::endian::little);

    using Self = Ipv6;


    // 通过字节数组构造
    inline constexpr explicit Ipv6(
        const std::span<const uint8_t, 16> bytes
    ){
        for(size_t i = 0; i < 16; ++i){
            this->bytes_[i] = bytes[i];
        }
    }

    #if 0
    // 从uint128构造（如果平台支持）
    static constexpr Self from_u128(const std::array<uint64_t, 2>& bits) { 
        return Self(bits); 
    }
    #endif

    template<size_t I> requires(I < 16)
    [[nodiscard]] constexpr uint8_t get_byte() const { 
        return bytes_[I]; 
    }

    template<size_t I> requires(I < 16)
    [[nodiscard]] constexpr uint8_t& get_byte() { 
        return bytes_[I]; 
    }

    template<size_t I> requires(I < 8)
    [[nodiscard]] constexpr uint16_t get_word() const { 
        return words_[I]; 
    }

    template<size_t I> requires(I < 8)
    [[nodiscard]] constexpr uint16_t& get_word() { 
        return words_[I]; 
    }

    [[nodiscard]] constexpr uint8_t operator [](const size_t idx) const {
        return bytes_[idx];
    }

    [[nodiscard]] constexpr uint8_t& operator [](const size_t idx) {
        return bytes_[idx];
    }

    [[nodiscard]] constexpr uint8_t at(const size_t idx) const {
        if(idx >= 16) [[unlikely]] __builtin_trap();
        return bytes_[idx];
    }

    [[nodiscard]] constexpr uint8_t& at(const size_t idx) {
        if(idx >= 16) [[unlikely]] __builtin_trap();
        return bytes_[idx];
    }

    [[nodiscard]] constexpr std::span<const uint8_t, 16> bytes() const { 
        return std::span(bytes_); 
    }
    
    [[nodiscard]] constexpr std::span<uint8_t, 16> bytes_mut() { 
        return std::span(bytes_); 
    }

    [[nodiscard]] constexpr std::span<const uint16_t, 8> words() const { 
        return std::span(words_); 
    }
    
    [[nodiscard]] constexpr std::span<uint16_t, 8> words_mut() { 
        return std::span(words_); 
    }

    [[nodiscard]] constexpr std::array<uint64_t, 2> to_u128() const { 
        std::array<uint64_t, 2> result;
        result[0] = (static_cast<uint64_t>(words_[0]) << 48) |
                (static_cast<uint64_t>(words_[1]) << 32) |
                (static_cast<uint64_t>(words_[2]) << 16) |
                (static_cast<uint64_t>(words_[3]));
        result[1] = (static_cast<uint64_t>(words_[4]) << 48) |
                (static_cast<uint64_t>(words_[5]) << 32) |
                (static_cast<uint64_t>(words_[6]) << 16) |
                (static_cast<uint64_t>(words_[7]));
        return result;
    }

    [[nodiscard]] constexpr bool operator ==(const Self& other) const {
        return bytes_ == other.bytes_;
    }

private:
    union {
        alignas(8) std::array<uint8_t, 16> bytes_;
        std::array<uint16_t, 8> words_;
    };

    #if 0
    inline constexpr explicit Ipv6(const std::array<uint64_t, 2>& bits) {
        // 处理字节序转换
        bytes_[0]  = (bits[0] >> 56) & 0xFF;
        bytes_[1]  = (bits[0] >> 48) & 0xFF;
        bytes_[2]  = (bits[0] >> 40) & 0xFF;
        bytes_[3]  = (bits[0] >> 32) & 0xFF;
        bytes_[4]  = (bits[0] >> 24) & 0xFF;
        bytes_[5]  = (bits[0] >> 16) & 0xFF;
        bytes_[6]  = (bits[0] >> 8)  & 0xFF;
        bytes_[7]  = bits[0] & 0xFF;
        bytes_[8]  = (bits[1] >> 56) & 0xFF;
        bytes_[9]  = (bits[1] >> 48) & 0xFF;
        bytes_[10] = (bits[1] >> 40) & 0xFF;
        bytes_[11] = (bits[1] >> 32) & 0xFF;
        bytes_[12] = (bits[1] >> 24) & 0xFF;
        bytes_[13] = (bits[1] >> 16) & 0xFF;
        bytes_[14] = (bits[1] >> 8)  & 0xFF;
        bytes_[15] = bits[1] & 0xFF;
    }
    #endif
};
}