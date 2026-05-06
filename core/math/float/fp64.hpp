#pragma once

#include <cstdint>
#include <bit>
#include <compare>
#include <span>

static_assert(8 == sizeof(double));
namespace ymd::math{

struct alignas(8) [[nodiscard]] fp64 final{
	using Self = fp64;

	uint64_t mant:52;
	uint64_t exp:11;
	uint64_t sign:1;

    constexpr fp64(double dv){
		*this = std::bit_cast<fp64>(dv);
	}

	constexpr fp64 operator -() const {
		return Self::from_bits(to_bits() ^ std::numeric_limits<uint64_t>::min());
	}

	[[nodiscard]] static constexpr Self from_bits(const uint64_t bits) {
		return std::bit_cast<Self>(bits);
	}

	[[nodiscard]] constexpr uint64_t to_bits() const {
		return std::bit_cast<uint64_t>(*this);
	}

	[[nodiscard]] consteval Self from_nan() const {
		return Self{0x7ff8000000000000};
	}

    [[nodiscard]] constexpr operator double() const {
        return std::bit_cast<double>(*this);
    }

	[[nodiscard]] constexpr std::partial_ordering operator <=>(const fp64 & other){
		return float(*this) <=> float(other);
	}


};

static_assert(sizeof(fp64) == sizeof(double));

}

namespace std{
	//建立元函数偏特化
    template<>
    struct is_arithmetic<ymd::math::fp64> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp64> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp64> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp64> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp64> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp64> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp64> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp64> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp64> {
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
        static constexpr int digits = 53;      // 1 + 52 bits
        static constexpr int digits10 = 15;
        static constexpr int max_digits10 = 17;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -1022;
        static constexpr int min_exponent10 = -308;
        static constexpr int max_exponent = 1023;
        static constexpr int max_exponent10 = 308;

        static constexpr ymd::math::fp64 min() noexcept { return ymd::math::fp64::from_bits(0x0010000000000000ULL); }
        static constexpr ymd::math::fp64 lowest() noexcept { return ymd::math::fp64::from_bits(0xFFEFFFFFFFFFFFFFULL); }
        static constexpr ymd::math::fp64 max() noexcept { return ymd::math::fp64::from_bits(0x7FEFFFFFFFFFFFFFULL); }
        static constexpr ymd::math::fp64 epsilon() noexcept { return ymd::math::fp64::from_bits(0x3CB0000000000000ULL); }
        static constexpr ymd::math::fp64 round_error() noexcept { return ymd::math::fp64(0.5); }
        static constexpr ymd::math::fp64 infinity() noexcept { return ymd::math::fp64::from_bits(0x7FF0000000000000ULL); }
        static constexpr ymd::math::fp64 quiet_NaN() noexcept { return ymd::math::fp64::from_bits(0x7FF8000000000000ULL); }
        static constexpr ymd::math::fp64 signaling_NaN() noexcept { return ymd::math::fp64::from_bits(0x7FF0000000000001ULL); }
        static constexpr ymd::math::fp64 denorm_min() noexcept { return ymd::math::fp64::from_bits(0x0000000000000001ULL); }
    };

}