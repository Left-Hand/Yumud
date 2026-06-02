#pragma once

#include "../briter_primitive.hpp"

namespace ymd::robots::briter{

enum class [[nodiscard]] CanCommand : uint8_t {
    Heartbeat = 0x00,
    SetCurrent = 0x01,
    SetAbsPos = 0x04,

    TrajMaxSpeed = 0x0b,
    TrajMaxAccel = 0x0c,
    TrajMaxDecel = 0x0d
};

enum class [[nodiscard]] CanEnqueryId:uint8_t{
    FindZSignal = 0x0a
};


}