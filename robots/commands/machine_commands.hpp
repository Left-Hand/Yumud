#pragma once

#include "core/utils/serde.hpp"

namespace ymd{
namespace robots::machine_commands{

struct RapidMoveXy{
    bf16 x;
    bf16 y;
};

struct RapidMoveXyz{
    bf16 x;
    bf16 y;
    bf16 z;
};

struct LinearMoveXy{
    bf16 x;
    bf16 y;
};

struct LinearMoveXyz{
    bf16 x;
    bf16 y;
    bf16 z;
};

struct Dwell{
    uint16_t dwell_ms;
};

struct Replace{
    bf16 x1, y1;
    bf16 x2, y2;
};


struct SpinAroundCenter{
    bf16 theta;
};

struct RotateAroundCenter{
    bf16 orientation;
};

struct Abort{
};
}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::RapidMoveXy)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::RapidMoveXy)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::RapidMoveXy)
DEF_DERIVE_MEM_REFLECTER_2(robots::machine_commands::RapidMoveXy, x, y)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::RapidMoveXyz)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::RapidMoveXyz)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::RapidMoveXyz)
DEF_DERIVE_MEM_REFLECTER_3(robots::machine_commands::RapidMoveXyz, x, y, z)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::LinearMoveXy)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::LinearMoveXy)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::LinearMoveXy)
DEF_DERIVE_MEM_REFLECTER_2(robots::machine_commands::LinearMoveXy, x, y)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::LinearMoveXyz)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::LinearMoveXyz)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::LinearMoveXyz)
DEF_DERIVE_MEM_REFLECTER_3(robots::machine_commands::LinearMoveXyz, x, y, z)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Dwell)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Dwell)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Dwell)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Replace)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Replace)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Replace)
DEF_DERIVE_MEM_REFLECTER_4(robots::machine_commands::Replace, x1, y1, x2, y2)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::SpinAroundCenter)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::SpinAroundCenter)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::SpinAroundCenter)
DEF_DERIVE_MEM_REFLECTER_1(robots::machine_commands::SpinAroundCenter, theta)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::RotateAroundCenter)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::RotateAroundCenter)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::RotateAroundCenter)
DEF_DERIVE_MEM_REFLECTER_1(robots::machine_commands::RotateAroundCenter, orientation)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Abort)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Abort)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Abort)
DEF_DERIVE_MEM_REFLECTER_0(robots::machine_commands::Abort)

}