#pragma once

#include <cstdint>
#include <bit>
#include <compare>
#include <span>

static_assert(8 == sizeof(double));
namespace ymd::math{

struct [[nodiscard]] fp64{
	using Self = fp64;

	uint64_t frac:52;
	uint64_t exp:11;
	uint64_t sign:1;

    constexpr fp64(double dv){
		*this = std::bit_cast<fp64>(dv);
	}

	[[nodiscard]] Self from_bits(const uint64_t bits) {
		return std::bit_cast<Self>(bits);
	}

	[[nodiscard]] uint64_t to_bits() const {
		return std::bit_cast<uint64_t>(*this);
	}

	[[nodiscard]] Self from_nan() const {
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