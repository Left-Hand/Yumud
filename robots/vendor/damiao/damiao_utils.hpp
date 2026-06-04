#pragma once

#include "core/math/float/fp32.hpp"

namespace ymd::robots::damiao::utils{

static constexpr uint8_t * ptr_push_fp32(uint8_t * cursor, const math::fp32 value){
    cursor[0] = uint8_t(std::bit_cast<uint32_t>(value) >> 0);
    cursor[1] = uint8_t(std::bit_cast<uint32_t>(value) >> 8);
    cursor[2] = uint8_t(std::bit_cast<uint32_t>(value) >> 16);
    cursor[3] = uint8_t(std::bit_cast<uint32_t>(value) >> 24);
    return cursor + 4;
}

static constexpr uint8_t * ptr_push_u16le(uint8_t * cursor, const uint16_t value){
    cursor[0] = uint8_t(value >> 0);
    cursor[1] = uint8_t(value >> 8);
    return cursor + 4;
}


}