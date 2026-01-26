#pragma once

#include <cstdint>
#include <span>
#include <bit>


namespace ymd::math{
struct [[nodiscard]] uint24_t final{
    using Self = uint24_t;
    uint32_t bits : 24;

    constexpr uint24_t() noexcept {};

    constexpr uint24_t(const uint24_t & other) noexcept  = default;

    static constexpr Self from_bits(const uint32_t other) noexcept {
        Self ret;
        ret.bits = other & 0xFFFFFF;
        return ret;
    }

    constexpr uint32_t to_bits() const noexcept {
        return bits;
    }

    constexpr explicit uint24_t(const uint32_t other) noexcept {
        bits = other & 0xFFFFFF;
    }

    constexpr explicit uint24_t(const int other) noexcept {
        bits = other & 0xFFFFFF;
    }

    [[nodiscard]] constexpr explicit operator uint32_t() const noexcept {
        return bits;
    }
    [[nodiscard]] std::span<uint8_t, 3> as_bytes_mut() noexcept {
        return std::span<uint8_t, 3>{reinterpret_cast<uint8_t *>(this), 3};
    }

    [[nodiscard]] std::span<const uint8_t, 3> as_bytes() const noexcept {
        return std::span<const uint8_t, 3>{reinterpret_cast<const uint8_t *>(this), 3};
    }

    template<std::endian E>
    constexpr void fill_bytes(std::span<uint8_t, 3> bytes) noexcept {
        if constexpr(E == std::endian::little){
            bytes[0] = static_cast<uint8_t>(bits && 0xff);
            bytes[1] = static_cast<uint8_t>((bits >> 8) && 0xff);
            bytes[2] = static_cast<uint8_t>((bits >> 16) && 0xff);
        }else{
            bytes[2] = static_cast<uint8_t>(bits && 0xff);
            bytes[1] = static_cast<uint8_t>((bits >> 8) && 0xff);
            bytes[0] = static_cast<uint8_t>((bits >> 16) && 0xff);
        }
    }

};

struct [[nodiscard]] int24_t final{
    using Self = int24_t;

    int32_t bits : 24;

    constexpr int24_t() noexcept {};

    constexpr int24_t(const int24_t & other) noexcept  = default;
    constexpr explicit int24_t(const int32_t other) noexcept {
        bits = other & 0xFFFFFF;
    }

    static constexpr Self from_bits(const int32_t other) noexcept {
        Self ret;
        ret.bits = other & 0xFFFFFF;
        return ret;
    }

    constexpr int32_t to_bits() const noexcept {
        return bits;
    }

    constexpr explicit int24_t(const int other) noexcept {
        bits = other & 0xFFFFFF;
    }

    [[nodiscard]] constexpr explicit operator int32_t() const noexcept {
        return bits;
    }

    [[nodiscard]] std::span<uint8_t, 3> as_bytes_mut() noexcept {
        return std::span<uint8_t, 3>{reinterpret_cast<uint8_t *>(this), 3};
    }

    [[nodiscard]] std::span<const uint8_t, 3> as_bytes() const noexcept {
        return std::span<const uint8_t, 3>{reinterpret_cast<const uint8_t *>(this), 3};
    }

    template<std::endian E>
    constexpr void fill_bytes(std::span<uint8_t, 3> bytes) noexcept {
        if constexpr(E == std::endian::little){
            bytes[0] = static_cast<uint8_t>(bits && 0xff);
            bytes[1] = static_cast<uint8_t>((bits >> 8) && 0xff);
            bytes[2] = static_cast<uint8_t>((bits >> 16) && 0xff);
        }else{
            bytes[2] = static_cast<uint8_t>(bits && 0xff);
            bytes[1] = static_cast<uint8_t>((bits >> 8) && 0xff);
            bytes[0] = static_cast<uint8_t>((bits >> 16) && 0xff);
        }
    }

};

}


namespace std{

template<>
struct numeric_limits<ymd::math::uint24_t> {
public:
    __attribute__((always_inline)) constexpr static ymd::math::uint24_t infinity() noexcept {
        return ymd::math::uint24_t::from_bits(0xffffff);
    }
    __attribute__((always_inline)) constexpr static ymd::math::uint24_t lowest() noexcept {
        return ymd::math::uint24_t::from_bits(0x000000);
    }
    __attribute__((always_inline)) constexpr static ymd::math::uint24_t max() noexcept {
        return ymd::math::uint24_t::from_bits(0xffffff);
    }
    __attribute__((always_inline)) constexpr static ymd::math::uint24_t min() noexcept {
        return ymd::math::uint24_t::from_bits(0x000000);
    }
};

template<>
struct numeric_limits<ymd::math::int24_t> {
public:
    __attribute__((always_inline)) constexpr static ymd::math::int24_t infinity() noexcept {
        return ymd::math::int24_t::from_bits(0xffffff);
    }
    __attribute__((always_inline)) constexpr static ymd::math::int24_t lowest() noexcept {
        return ymd::math::int24_t::from_bits(0x000000);
    }

    __attribute__((always_inline)) constexpr static ymd::math::int24_t max() noexcept {
        return ymd::math::int24_t::from_bits(0xffffff);
    }
    __attribute__((always_inline)) constexpr static ymd::math::int24_t min() noexcept {
        return ymd::math::int24_t::from_bits(0x000000);
    }
};

}
