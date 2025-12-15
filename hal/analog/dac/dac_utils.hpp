#pragma once

#include <cstdint>

namespace ymd::hal::dac{

enum class ChannelSelection:uint8_t{
    _1,
    _2
};

enum class Alignment:uint8_t{
    R12 = ((uint32_t)0x00000000),
    L12 = ((uint32_t)0x00000004),
    R8 = ((uint32_t)0x00000008)
};

}