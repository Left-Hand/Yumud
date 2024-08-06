#pragma once

#include "types/real.hpp"

#include <cstdint>
#include <bit>

struct fp32{
	union {
		uint32_t raw;
		struct {
			uint32_t frac:23;
			uint32_t exp:8;
			uint32_t sign:1;
		} __attribute__((__packed__));
	};

    constexpr fp32(float fv) : raw(std::bit_cast<uint32_t>(fv)) {}

    constexpr operator float() const {
        return std::bit_cast<float>(raw);
    }
};
