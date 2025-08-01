#pragma once

#include <cstdint>
#include "core/stream/ostream.hpp"

namespace ymd::robots{

enum class NodeRole:uint8_t{
    YawJoint = 0x01,
    RollJoint,
    PitchJoint,
    Laser,
    // YawJoint,
    // AxisX,
    // AxisY,
    // AxisZ,
    LeftWheel,
    RightWheel,
    // Computer = 0x0f,
};

}

namespace ymd{

DEF_DERIVE_DEBUG(robots::NodeRole)
}