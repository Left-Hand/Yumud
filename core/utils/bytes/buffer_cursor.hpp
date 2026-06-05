#pragma once

#include <cstdint>
#include <bit>
#include "core/tmp/bits/width.hpp"

namespace ymd{

template<typename T, std::endian ENDIAN, typename U>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_int(uint8_t * ptr, U val){
    if constexpr (ENDIAN != std::endian::native) {
        val = std::byteswap(val);
    }
    for (size_t i = 0; i < sizeof(T); ++i) {
        ptr[i] = static_cast<uint8_t>(val >> (i * 8));
    }
    return ptr + sizeof(T);
}

template<typename T, std::endian ENDIAN, typename U>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_bits(uint8_t * ptr, U val) 
    requires (
        std::is_trivially_copyable_v<T> &&
        std::is_trivially_copyable_v<U> &&
        sizeof(T) == sizeof(U)
    )
{
    return u8ptr_push_int<T, ENDIAN>(ptr, std::bit_cast<T>(val));
}

template<std::endian ENDIAN, typename U>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_bits_auto(uint8_t * ptr, U val) 
{
    using T = tmp::size_to_uint_t<sizeof(U)>;
    return u8ptr_push_int<T, ENDIAN>(ptr, std::bit_cast<T>(val));
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u64be(uint8_t * ptr, uint64_t val){
    return u8ptr_push_int<uint64_t, std::endian::big>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u32be(uint8_t * ptr, uint32_t val){
    return u8ptr_push_int<uint32_t, std::endian::big>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u16be(uint8_t * ptr, uint16_t val){
    return u8ptr_push_int<uint16_t, std::endian::big>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u8be(uint8_t * ptr, uint8_t val){
    return u8ptr_push_int<uint8_t, std::endian::big>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u64le(uint8_t * ptr, uint64_t val){
    return u8ptr_push_int<uint64_t, std::endian::little>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u32le(uint8_t * ptr, uint32_t val){
    return u8ptr_push_int<uint32_t, std::endian::little>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u16le(uint8_t * ptr, uint16_t val){
    return u8ptr_push_int<uint16_t, std::endian::little>(ptr, val);
}

__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u8le(uint8_t * ptr, uint8_t val){
    return u8ptr_push_int<uint8_t, std::endian::little>(ptr, val);
}

template<typename T, std::endian ENDIAN>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr T
u8ptr_load_int(const uint8_t* ptr) {
    T val;
    for (size_t i = 0; i < sizeof(T); ++i) {
        val |= static_cast<T>(ptr[i]) << (i * 8);
    }
    if constexpr (ENDIAN != std::endian::native) {
        val = std::byteswap(val);
    }
    return val;
}


template<typename T, std::endian ENDIAN>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr T
u8ptr_pop_int(const uint8_t*& ptr) {
    T val = u8ptr_load_int<T, ENDIAN>(ptr);
    ptr += sizeof(T);
    return val;
}

template<typename T, std::endian ENDIAN>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr T
u8ptr_load_bits(const uint8_t* ptr)
{
    return std::bit_cast<T>(u8ptr_read_int<T, ENDIAN>(ptr));
}

template<typename T, std::endian ENDIAN, typename U>
__attribute__((always_inline, optimize("Ofast")))
[[nodiscard]] static constexpr U
u8ptr_pop_bits(const uint8_t*& ptr)
    requires (
        std::is_trivially_copyable_v<T> &&
        std::is_trivially_copyable_v<U> &&
        sizeof(T) == sizeof(U)
    )
{
    U val = u8ptr_read_bits<T, ENDIAN, U>(ptr);
    ptr += sizeof(T);
    return val;
}

struct [[nodiscard]] BufferCursor final{
    uint8_t * ptr;

    template<typename T, std::endian ENDIAN, typename U>
    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_int(const U val){
        ptr = u8ptr_push_int<T, ENDIAN, U>(ptr, val);
    }

    template<typename T, std::endian ENDIAN, typename U>
    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_bits(const U val){
        ptr = u8ptr_push_bits<T, ENDIAN, U>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u64be(const uint64_t val) {
        ptr = u8ptr_push_int<uint64_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u32be(const uint32_t val) {
        ptr = u8ptr_push_int<uint32_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u16be(const uint16_t val) {
        ptr = u8ptr_push_int<uint16_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u8be(const uint8_t val) {
        ptr = u8ptr_push_int<uint8_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u64le(const uint64_t val) {
        ptr = u8ptr_push_int<uint64_t, std::endian::little>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u32le(const uint32_t val) {
        ptr = u8ptr_push_int<uint32_t, std::endian::little>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_u16le(const uint16_t val) {
        ptr = u8ptr_push_int<uint16_t, std::endian::little>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b64be(const auto val) {
        ptr = u8ptr_push_bits<uint64_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b32be(const auto val) {
        ptr = u8ptr_push_bits<uint32_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b16be(const auto val) {
        ptr = u8ptr_push_bits<uint16_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b8be(const auto val) {
        ptr = u8ptr_push_bits<uint8_t, std::endian::big>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b64le(const auto val) {
        ptr = u8ptr_push_bits<uint64_t, std::endian::little>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b32le(const auto val) {
        ptr = u8ptr_push_bits<uint32_t, std::endian::little>(ptr, val);
    }

    __attribute__((always_inline, optimize("Ofast")))
    constexpr void push_b16le(const auto val) {
        ptr = u8ptr_push_bits<uint16_t, std::endian::little>(ptr, val);
    }
};

}