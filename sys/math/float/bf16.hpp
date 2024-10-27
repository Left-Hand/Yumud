#pragma once

#include "fp32.hpp"
#include "math/real.hpp"

#include <cstdint>
#include <bit>


struct bf16 {
	union {
		uint16_t raw;
		struct {
			uint16_t frac:7;
			uint16_t exp:8;
			uint16_t sign:1;
        };
	};

    constexpr bf16(){;}

    constexpr bf16 & operator = (const bf16 & other){
        raw = other.raw;
        return *this;
    }

    constexpr bf16(const bf16 & other):raw(other.raw){;}
    constexpr bf16(bf16 && other):raw(other.raw){;}
    constexpr bf16(float fv) : raw(std::bit_cast<uint32_t>(fv) >> 16) {}
    constexpr bf16(iq_t qv): bf16(float(qv)) {;}
    constexpr bf16(int iv) : bf16(float(iv)) {}

    explicit constexpr operator float() const {
        return std::bit_cast<float>(uint32_t(raw) << 16); 
    }

    explicit constexpr operator int() const {
        return int(float(*this));
    }

    constexpr operator real_t() const {
        return real_t::from(float(*this));
    }
};