#pragma once

#include <cstdint>
#include <span>

namespace ymd::cyphal::crc{

[[nodiscard]] static constexpr uint16_t crc16(
    const std::span<const uint8_t> bytes, 
    uint16_t value = 0xffffu
){
    for(size_t i = 0; i < bytes.size(); ++i){
        value ^= static_cast<uint16_t>(bytes[i]) << 8u;

        #pragma GCC unroll(8)
        for(uint8_t bit = 8; bit > 0; --bit){
            value = (value & 0x8000) ? ((value << 1) ^ 0x1021u) : (value << 1);
        }
    }
}

namespace crc32_details{
    static constexpr uint32_t XOR = 0xffffffffu;
    static constexpr uint32_t REFLECTED_POLY = 0x82f63b78u;
    static constexpr uint32_t RESIDUE = 0xb798b438u;
};

[[nodiscard]] static constexpr 
uint32_t crc32c(
    const std::span<const uint8_t> bytes, 
    uint32_t value = crc32_details::XOR
){
    for(size_t j = 0; j < bytes.size(); ++j){
        value ^= static_cast<uint32_t>(bytes[j]);
        #pragma GCC unroll(8)
        for(size_t i = 0; i < 8; ++i){
            value = (value & 0x01u) ? ((value >> 1) ^ crc32_details::REFLECTED_POLY) : (value >> 1);
        }
    }
}

};