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
    uint16_t mant:7;
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
};

static_assert(sizeof(bf16) == 2);

consteval bf16 operator""_bf16(long double x){
    return bf16(x);
}

consteval bf16 operator""_bf16(unsigned long long x){
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
    template<>
    struct is_integral<ymd::math::bf16> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::bf16> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::bf16> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::bf16> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::bf16> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::bf16> {
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = true;
        static constexpr bool has_signaling_NaN = true;
        static constexpr std::float_denorm_style has_denorm = std::denorm_present;
        static constexpr bool has_denorm_loss = false;
        static constexpr std::float_round_style round_style = std::round_to_nearest;
        static constexpr bool is_iec559 = true;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 8;      // 1 + 7 bits
        static constexpr int digits10 = 2;
        static constexpr int max_digits10 = 4;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -126;
        static constexpr int min_exponent10 = -38;
        static constexpr int max_exponent = 127;
        static constexpr int max_exponent10 = 38;

        static constexpr ymd::math::bf16 min() noexcept { return ymd::math::bf16::from_bits(0x0080); }
        static constexpr ymd::math::bf16 lowest() noexcept { return ymd::math::bf16::from_bits(0xFF7F); }
        static constexpr ymd::math::bf16 max() noexcept { return ymd::math::bf16::from_bits(0x7F7F); }
        static constexpr ymd::math::bf16 epsilon() noexcept { return ymd::math::bf16::from_bits(0x3C00); }
        static constexpr ymd::math::bf16 round_error() noexcept { return ymd::math::bf16(0.5f); }
        static constexpr ymd::math::bf16 infinity() noexcept { return ymd::math::bf16::from_bits(0x7F80); }
        static constexpr ymd::math::bf16 quiet_NaN() noexcept { return ymd::math::bf16::from_bits(0x7FC0); }
        static constexpr ymd::math::bf16 signaling_NaN() noexcept { return ymd::math::bf16::from_bits(0x7F81); }
        static constexpr ymd::math::bf16 denorm_min() noexcept { return ymd::math::bf16::from_bits(0x0001); }
    };

}