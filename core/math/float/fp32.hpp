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

	uint32_t frac:23;
	uint32_t exp:8;
	uint32_t sign:1;

    constexpr fp32() = default;
    constexpr fp32(float fv){
		*this = std::bit_cast<fp32>(fv);
	}

	static constexpr fp32 from_parts(const uint32_t frac, const uint32_t exp, const uint32_t sign){
		Self self;
		self.frac = frac;
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
}
