#pragma once

#include <cstdint>
#include <bit>
#include <compare>
#include <span>

static_assert(sizeof(float) == 4);
namespace ymd{
class OutputStream;

namespace math{
struct alignas(4) [[nodiscard]] fp32 final{
	using Self = fp32;

	uint32_t mant:23;
	uint32_t exp:8;
	uint32_t sign:1;

    constexpr fp32() = default;
    constexpr fp32(float fv){
		*this = std::bit_cast<fp32>(fv);
	}

	static constexpr fp32 from_parts(const uint32_t mant, const uint32_t exp, const uint32_t sign){
		Self self;
		self.mant = mant;
		self.exp = exp;
		self.sign = sign;
		return self;
	}

	constexpr fp32 operator -() const {
		return Self::from_bits(this->to_bits() ^ 0x80000000);
	}

	static constexpr fp32 from_bits(const uint32_t bits){
		return fp32(std::bit_cast<float>(bits));
	}

	static constexpr fp32 from_le_bytes(const std::span<const uint8_t, 4> bytes){
		std::array<uint8_t, 4> bytes_arr;
		std::copy(bytes.begin(), bytes.end(), bytes_arr.begin());
		return fp32::from_bits(std::bit_cast<uint32_t>(bytes_arr));
	}

	[[nodiscard]] constexpr uint32_t to_bits() const {
		return std::bit_cast<uint32_t>(*this);
	}

	[[nodiscard]] constexpr std::partial_ordering operator <=>(const fp32 & other){
		return float(*this) <=> float(other);
	}

	[[nodiscard]] constexpr bool is_nan() const{
		return std::bit_cast<uint32_t>(*this) == 0x7fc00000;
	}

	[[nodiscard]] static consteval Self from_nan(){
		return Self::from_bits(0x7fc00000);
	}

    [[nodiscard]] explicit constexpr operator float() const {
        return std::bit_cast<float>(*this);
    }

};

}
OutputStream & operator << (OutputStream & os, const math::fp32 v);
}

namespace std{
	//建立元函数偏特化
    template<>
    struct is_arithmetic<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_integral<ymd::math::fp32> : std::false_type {};
    template<>
    struct is_scalar<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_trivially_copyable<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_trivially_destructible<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_trivially_default_constructible<ymd::math::fp32> : std::true_type {};

    template<>
    struct numeric_limits<ymd::math::fp32> {
        static constexpr bool is_specialized = true;
        static constexpr bool is_signed = true;
        static constexpr bool is_integer = false;
        static constexpr bool is_exact = false;
        static constexpr bool has_infinity = true;
        static constexpr bool has_quiet_NaN = true;
        static constexpr bool has_signaling_NaN = true;
        static constexpr bool has_denorm_loss = false;
        static constexpr bool is_iec559 = true;
        static constexpr bool is_bounded = true;
        static constexpr bool is_modulo = false;
        static constexpr int digits = 24;      // 1 + 23 bits
        static constexpr int digits10 = 6;
        static constexpr int max_digits10 = 9;
        static constexpr int radix = 2;
        static constexpr int min_exponent = -126;
        static constexpr int min_exponent10 = -38;
        static constexpr int max_exponent = 127;
        static constexpr int max_exponent10 = 38;

        static constexpr ymd::math::fp32 min() noexcept { return ymd::math::fp32::from_bits(0x00800000); }
        static constexpr ymd::math::fp32 lowest() noexcept { return ymd::math::fp32::from_bits(0xFF7FFFFF); }
        static constexpr ymd::math::fp32 max() noexcept { return ymd::math::fp32::from_bits(0x7F7FFFFF); }
        static constexpr ymd::math::fp32 epsilon() noexcept { return ymd::math::fp32::from_bits(0x34000000); }
        static constexpr ymd::math::fp32 round_error() noexcept { return ymd::math::fp32(0.5f); }
        static constexpr ymd::math::fp32 infinity() noexcept { return ymd::math::fp32::from_bits(0x7F800000); }
        static constexpr ymd::math::fp32 quiet_NaN() noexcept { return ymd::math::fp32::from_bits(0x7FC00000); }
        static constexpr ymd::math::fp32 signaling_NaN() noexcept { return ymd::math::fp32::from_bits(0x7F800001); }
        static constexpr ymd::math::fp32 denorm_min() noexcept { return ymd::math::fp32::from_bits(0x00000001); }
    };

}