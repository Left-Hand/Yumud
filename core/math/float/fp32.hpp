#pragma once

#include <cstdint>
#include <bit>

struct fp32{
	uint32_t frac:23;
	uint32_t exp:8;
	uint32_t sign:1;


    constexpr fp32(float fv): 
		frac(std::bit_cast<uint32_t>(fv)),
		exp(std::bit_cast<uint32_t>(fv)>>23 & 0xff),
		sign(std::bit_cast<uint32_t>(fv)>>31){;}

    constexpr operator float() const {
        return std::bit_cast<float>(*this);
    }
};
