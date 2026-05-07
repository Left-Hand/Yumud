#pragma once

#include "core/utils/bits/u11x16.hpp"

namespace ymd::robots::flyskytech::fsi6x{



enum class [[nodiscard]] ConnectionCode:uint8_t{
    Connected = 0x00,
    Disconnected = 0x0c
};

#pragma pack(push, 1)

struct Channels{
    U11X16Owned elements;

};

struct [[nodiscard]] Packet final{

    Channels channels;

    ConnectionCode connection_code;
};

#pragma pack(pop)

static_assert(sizeof(Packet) == 23);

}