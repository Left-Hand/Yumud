#pragma once

#include <cstdint>
#include <bit>

namespace ymd::math{
struct fp64{
	uint64_t frac:52;
	uint64_t exp:11;
	uint64_t sign:1;

    constexpr fp64(double dv): 
		frac(std::bit_cast<uint64_t>(dv) & 0xfffffffffffff),
		exp((std::bit_cast<uint64_t>(dv) >> 52) & 0x7ff),
		sign(std::bit_cast<uint64_t>(dv) >> 63){;}

    constexpr operator double() const {
        return std::bit_cast<double>(*this);
    }
};
}