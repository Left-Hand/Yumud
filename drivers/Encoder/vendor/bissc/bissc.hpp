#pragma once

#include <cstdint>
#include <array>
#include "core/utils/Option.hpp"

namespace ymd::drivers::bissc{


static constexpr std::array<uint8_t, 64> CRC6_TABLE = []{
    constexpr uint8_t POLY = 0x43;
    std::array<uint8_t, 64> table;
	for(size_t i = 0; i < 64; i++){
		int crc = i;

		for (int j = 0; j < 6; j++){
			if (crc & 0x20){
				crc <<= 1;
				crc ^= POLY;
			} else {
				crc <<= 1;
			}
		}
		table[i] = crc;
	}
    return table;
}();

static constexpr uint8_t fast_clzll(const uint64_t x){
    const uint32_t low = x & 0xFFFFFFFF;
    const uint32_t high = x >> 32;
    return low ? __builtin_clz(low) : __builtin_clz(high) + 32;
}

static constexpr Option<uint32_t> parse(std::span<const uint8_t> bytes, const size_t resolution){
    uint64_t d64;
    d64 = static_cast<uint64_t>(bytes[0]) << 56;
    d64 |= static_cast<uint64_t>(bytes[1]) << 48;
    d64 |= static_cast<uint64_t>(bytes[2]) << 40;
    d64 |= static_cast<uint64_t>(bytes[3]) << 32;
    d64 |= static_cast<uint64_t>(bytes[4]) << 24;
    d64 |= static_cast<uint64_t>(bytes[5]) << 16;
    d64 |= static_cast<uint64_t>(bytes[6]) << 8;
    d64 |= static_cast<uint64_t>(bytes[7]);

    d64 <<= fast_clzll(d64);
    d64 &= 0x3FFFFFFFFFFFFFFF;

    const size_t num_bit =  64-fast_clzll(d64);
    if ( num_bit >= (resolution + 10) ) {
        d64 >>= num_bit-(resolution + 10);
    }

    const uint8_t crc_expected = d64 & 0x3F;
    const uint32_t d32 = (d64 >> 6) & ((1<<(resolution + 2)) - 1);
    const uint32_t spi_val = (d32 >> 2) & ((1<<resolution) - 1);

    uint8_t crc_actual;  //CRC seed is 0b000000
    crc_actual = ((d32 >> 30) & 0x03);
    crc_actual = CRC6_TABLE[static_cast<size_t>(((d32 >> 24) & 0x3F) ^ crc_actual)];
    crc_actual = CRC6_TABLE[static_cast<size_t>(((d32 >> 18) & 0x3F) ^ crc_actual)];
    crc_actual = CRC6_TABLE[static_cast<size_t>(((d32 >> 12) & 0x3F) ^ crc_actual)];
    crc_actual = CRC6_TABLE[static_cast<size_t>(((d32 >> 6) & 0x3F) ^ crc_actual)];
    crc_actual = CRC6_TABLE[static_cast<size_t>(((d32 >> 0) & 0x3F) ^ crc_actual)];
    crc_actual = 0x3F & ~crc_actual; //CRC is output inverted

    if(crc_actual != crc_expected) return None;
    return Some(spi_val << (32 - resolution));
}

}