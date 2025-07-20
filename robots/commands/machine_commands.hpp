#pragma once

#include "core/utils/serde.hpp"

namespace ymd{
namespace robots::machine_commands{

struct SetPositionXy{
    bf16 x;
    bf16 y;
};

struct SetPositionXyz{
    bf16 x;
    bf16 y;
    bf16 z;
};


struct SetPolar{
    bf16 radius;
    bf16 phi;
};


struct Replace{
    bf16 x1, y1;
    bf16 x2, y2;
};


struct Spin{
    bf16 delta_rotation;
};

struct Rotate{
    bf16 rotation;
};

struct Abort{
};
}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::SetPositionXy)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::SetPositionXy)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::SetPositionXy)
DEF_DERIVE_MEM_REFLECTER_2(robots::machine_commands::SetPositionXy, x, y)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::SetPositionXyz)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::SetPositionXyz)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::SetPositionXyz)
DEF_DERIVE_MEM_REFLECTER_3(robots::machine_commands::SetPositionXyz, x, y, z)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Replace)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Replace)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Replace)
DEF_DERIVE_MEM_REFLECTER_4(robots::machine_commands::Replace, x1, y1, x2, y2)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Spin)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Spin)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Spin)
DEF_DERIVE_MEM_REFLECTER_1(robots::machine_commands::Spin, delta_rotation)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Rotate)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Rotate)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Rotate)
DEF_DERIVE_MEM_REFLECTER_1(robots::machine_commands::Rotate, rotation)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_commands::Abort)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_commands::Abort)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_commands::Abort)
DEF_DERIVE_MEM_REFLECTER_0(robots::machine_commands::Abort)

}