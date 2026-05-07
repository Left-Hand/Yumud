#pragma once

#include "core/utils/serde/serde.hpp"

namespace ymd{
namespace robots::machine_msgs{

struct [[nodiscard]] RapidMoveXy{
    math::bf16 x;
    math::bf16 y;
};

struct [[nodiscard]] RapidMoveXyz{
    math::bf16 x;
    math::bf16 y;
    math::bf16 z;
};

struct [[nodiscard]] LinearMoveXy{
    math::bf16 x;
    math::bf16 y;
};

struct [[nodiscard]] LinearMoveXyz{
    math::bf16 x;
    math::bf16 y;
    math::bf16 z;
};

struct [[nodiscard]] Dwell{
    uint16_t dwell_ms;
};

struct [[nodiscard]] Replace{
    math::bf16 x1, y1;
    math::bf16 x2, y2;
};


struct [[nodiscard]] SpinAroundCenter{
    math::bf16 theta;
};

struct [[nodiscard]] RotateAroundCenter{
    math::bf16 orientation;
};

struct [[nodiscard]] Abort{
};
}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::RapidMoveXy)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::RapidMoveXy)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::RapidMoveXy)
DEF_DERIVE_MEM_REFLECTER_2(robots::machine_msgs::RapidMoveXy, x, y)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::RapidMoveXyz)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::RapidMoveXyz)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::RapidMoveXyz)
DEF_DERIVE_MEM_REFLECTER_3(robots::machine_msgs::RapidMoveXyz, x, y, z)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::LinearMoveXy)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::LinearMoveXy)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::LinearMoveXy)
DEF_DERIVE_MEM_REFLECTER_2(robots::machine_msgs::LinearMoveXy, x, y)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::LinearMoveXyz)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::LinearMoveXyz)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::LinearMoveXyz)
DEF_DERIVE_MEM_REFLECTER_3(robots::machine_msgs::LinearMoveXyz, x, y, z)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::Dwell)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::Dwell)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::Dwell)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::Replace)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::Replace)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::Replace)
DEF_DERIVE_MEM_REFLECTER_4(robots::machine_msgs::Replace, x1, y1, x2, y2)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::SpinAroundCenter)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::SpinAroundCenter)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::SpinAroundCenter)
DEF_DERIVE_MEM_REFLECTER_1(robots::machine_msgs::SpinAroundCenter, theta)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::RotateAroundCenter)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::RotateAroundCenter)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::RotateAroundCenter)
DEF_DERIVE_MEM_REFLECTER_1(robots::machine_msgs::RotateAroundCenter, orientation)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::machine_msgs::Abort)
DEF_DERIVE_DEBUG_AS_DISPLAY(robots::machine_msgs::Abort)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::machine_msgs::Abort)
DEF_DERIVE_MEM_REFLECTER_0(robots::machine_msgs::Abort)

}