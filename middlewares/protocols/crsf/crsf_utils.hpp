#pragma once

#include "crsf_tables.hpp"
#include "crsf_constants.hpp"
#include "core/string/view/string_view.hpp"
#include "core/string/utils/c_style/strnlen.hpp"


namespace ymd::crsf{

static constexpr uint8_t calc_crc(std::span<const uint8_t> bytes, uint8_t last = 0){
    uint8_t crc = last;
    #pragma GCC unroll 8
    for(size_t i = 0; i < bytes.size(); i++){
        crc = CRC8_TABLE[crc ^ bytes[i]];
    }
    return crc;
}

static constexpr uint8_t calc_command_crc(std::span<const uint8_t> bytes, uint8_t last = 0){
    uint8_t crc = last;
    #pragma GCC unroll 8
    for(size_t i = 0; i < bytes.size(); i++){
        crc = COMMAND_CRC8_TABLE[crc ^ bytes[i]];
    }
    return crc;
}

template<size_t Extents>
using CharsSlice = std::span<const uint8_t, Extents>;

// using CharsNullTerminated = std::span<const uint8_t>;




}