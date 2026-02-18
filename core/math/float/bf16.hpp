#pragma once

#include "fp32.hpp"
#include "core/math/real.hpp"

#include <cstdint>
#include <bit>
#include <array>
#include <span>

namespace ymd{
class OutputStream;
    
namespace math{


struct alignas(2) [[nodiscard]] bf16 final{
    uint16_t frac:7;
    uint16_t exp:8;
    uint16_t sign:1;


    constexpr bf16() = default;
    constexpr bf16(const bf16 & other) = default;
    constexpr bf16(bf16 && other) = default;
    constexpr bf16& operator=(const bf16&) = default;
    constexpr bf16& operator=(bf16 &&) = default;


    template<typename T>
    requires(std::is_floating_point_v<T>)
    constexpr bf16(T f_val){
        *this = from_bits(static_cast<uint16_t>(std::bit_cast<uint32_t>(float(f_val)) >> 16));
    }

    template<size_t Q>
    constexpr bf16(fixed<Q, int32_t> qv) : bf16(float(qv)) {}
    constexpr bf16(int int_val) : bf16(float(int_val)) {}
    constexpr bf16 operator -() const{
        return from_bits(to_bits() ^ 0x8000);
    }

    static constexpr bf16 from_bits(const uint16_t bits){
        return std::bit_cast<bf16>(bits);
    }
    
    [[nodiscard]] constexpr uint16_t to_bits() const {
        return std::bit_cast<uint16_t>(*this);
    }



    // bf16 -> float
    [[nodiscard]] explicit constexpr operator float() const {
        uint32_t f32_bits = uint32_t(to_bits()) << 16;
        return std::bit_cast<float>(f32_bits);
    }

    [[nodiscard]] explicit constexpr operator int() const {
        return int(float(*this));
    }

    template <size_t Q>
    [[nodiscard]] explicit constexpr operator fixed<Q, int32_t>() const{
        return fixed<Q, int32_t>::from(float(*this));
    }

    [[nodiscard]] constexpr std::array<uint8_t, 2> to_bytes() const {
        const uint16_t bits = to_bits();
        return std::bit_cast<std::array<uint8_t, 2>>(bits);
    }

    [[nodiscard]] std::span<const uint8_t, 2> as_bytes() const {
        return std::span<const uint8_t, 2>(
            reinterpret_cast<const uint8_t *>(this), 2
        );
    }

    template<std::endian ENDIAN>
    constexpr void fill_bytes(std::span<uint8_t, 2> bytes){
        if constexpr(ENDIAN == std::endian::little){
            bytes[0] = frac;
            bytes[1] = exp | (sign << 7);
        }else{
            bytes[0] = exp | (sign << 7);
            bytes[1] = frac;
        }
    }

};

static_assert(sizeof(bf16) == 2);

consteval bf16 operator"" _bf16(long double x){
    return bf16(x);
}

consteval bf16 operator"" _bf16(unsigned long long x){
    return bf16(x);
}

}

OutputStream & operator << (OutputStream & os, const math::bf16 v);
}


namespace std{

    template<>
    struct is_arithmetic<ymd::math::bf16> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::bf16> : std::true_type {};
    template<>
    struct is_signed<ymd::math::bf16> : std::true_type {};
}
