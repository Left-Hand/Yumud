#pragma once

#include <cstdint>
#include "core/stream/ostream.hpp"

namespace ymd::robots{

enum class NodeRole:uint8_t{
    YawJoint = 0x01,
    // RollJoint,
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

enum class LaserCommand:uint8_t{
    On = 0x33,
    Off
};

namespace ymd{

DEF_DERIVE_DEBUG(robots::NodeRole)
}