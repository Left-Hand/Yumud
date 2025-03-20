#pragma once

#include <cstdint>
#include <bit>

struct fp64{
	struct {
		uint64_t frac:52;
		uint64_t exp:11;
		uint64_t sign:1;
	}raw;

    constexpr fp64(double dv): 
		raw(std::bit_cast<uint64_t>(dv)){}

    constexpr operator double() const {
        return std::bit_cast<double>(raw);
    }
};
