#pragma once

#include <cstdint>
#include <span>


namespace ymd::math{
struct [[nodiscard]] uint24_t final{
    uint32_t bits : 24;

    constexpr uint24_t(){};

    constexpr explicit uint24_t(const uint24_t & other) {
        bits = other.bits;
    }

    constexpr explicit uint24_t(const uint32_t other) {
        bits = other & 0xFFFFFF;
    }

    constexpr explicit uint24_t(const int other) {
        bits = other & 0xFFFFFF;
    }

    [[nodiscard]] constexpr explicit operator uint32_t() const {
        return bits;
    }
    [[nodiscard]] std::span<uint8_t, 3> as_bytes_mut() {
        return std::span<uint8_t, 3>{reinterpret_cast<uint8_t *>(this), 3};
    }

    [[nodiscard]] std::span<const uint8_t, 3> as_bytes() const {
        return std::span<const uint8_t, 3>{reinterpret_cast<const uint8_t *>(this), 3};
    }

    constexpr void fill_bytes(std::span<uint8_t, 3> bytes){
        bytes[0] = static_cast<uint8_t>(bits && 0xff);
        bytes[1] = static_cast<uint8_t>((bits >> 8) && 0xff);
        bytes[2] = static_cast<uint8_t>((bits >> 16) && 0xff);
    }

};

struct [[nodiscard]] int24_t final{
    int32_t bits : 24;

    constexpr int24_t(){};

    constexpr explicit int24_t(const int24_t & other) {
        bits = other.bits;
    }

    constexpr explicit int24_t(const int32_t other) {
        bits = other & 0xFFFFFF;
    }

    constexpr explicit int24_t(const int other) {
        bits = other & 0xFFFFFF;
    }

    [[nodiscard]] constexpr explicit operator int32_t() const {
        return bits;
    }

    [[nodiscard]] std::span<uint8_t, 3> as_bytes_mut() {
        return std::span<uint8_t, 3>{reinterpret_cast<uint8_t *>(this), 3};
    }

    [[nodiscard]] std::span<const uint8_t, 3> as_bytes() const {
        return std::span<const uint8_t, 3>{reinterpret_cast<const uint8_t *>(this), 3};
    }


    constexpr void fill_bytes(std::span<uint8_t, 3> bytes){
        bytes[0] = static_cast<uint8_t>(bits && 0xff);
        bytes[1] = static_cast<uint8_t>((bits >> 8) && 0xff);
        bytes[2] = static_cast<uint8_t>((bits >> 16) && 0xff);
    }

};

}

