#pragma once

#include <cstdint>
#include <bit>
#include <compare>
#include <span>
#include <array>

static_assert(sizeof(float) == 4);


namespace ymd::math{
struct [[nodiscard]] fp32{
	uint32_t frac:23;
	uint32_t exp:8;
	uint32_t sign:1;

    constexpr fp32(float fv){
		*this = std::bit_cast<fp32>(fv);
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

	[[nodiscard]] constexpr std::array<uint8_t, 4> to_le_bytes() const {
		return std::bit_cast<std::array<uint8_t, 4>>(*this);
	}

    [[nodiscard]] constexpr operator float() const {
        return std::bit_cast<float>(*this);
    }

	[[nodiscard]] constexpr auto operator <=>(const fp32 & other){
		return float(*this) <=> float(other);
	}
};
}

namespace std{

    template<>
    struct is_arithmetic<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_floating_point<ymd::math::fp32> : std::true_type {};
    template<>
    struct is_signed<ymd::math::fp32> : std::true_type {};
}
