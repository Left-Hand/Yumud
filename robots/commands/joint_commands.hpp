#pragma once

#include "core/utils/serde.hpp"

namespace ymd{


namespace robots::joint_commands{

struct SetPosition{
    bf16 position;
};


struct SetSpeed{
    bf16 speed;
};


struct SetPositionWithFwdSpeed{
    bf16 position;
    bf16 speed;
};


struct SetTrapzoid{
    bf16 position;
    bf16 speed;
    bf16 acceleration;
    bf16 deceleration;
};


struct SetKpKd{
    bf16 kp;
    bf16 kd;
};


struct SetKpKiKd{
    bf16 kp;
    bf16 ki;
    bf16 kd;
};

struct Deactivate{

};

struct Activate{

};

struct StartSeeking{

};

struct StopTracking{

};

struct PerspectiveRectInfo{
    // std::array<Vector2<uint8_t>, 4> points;
    std::array<uint8_t, 8> data;
};

}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetPosition)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetPosition)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::SetPosition, position)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetSpeed)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::SetSpeed, speed)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetPositionWithFwdSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetPositionWithFwdSpeed)
DEF_DERIVE_MEM_REFLECTER_2(robots::joint_commands::SetPositionWithFwdSpeed, position, speed)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetTrapzoid)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetTrapzoid)
DEF_DERIVE_MEM_REFLECTER_4(robots::joint_commands::SetTrapzoid, position, speed, acceleration, deceleration)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetKpKd)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetKpKd)
DEF_DERIVE_MEM_REFLECTER_2(robots::joint_commands::SetKpKd, kp, kd)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetKpKiKd)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetKpKiKd)
DEF_DERIVE_MEM_REFLECTER_3(robots::joint_commands::SetKpKiKd, kp, ki, kd)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::Deactivate)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::Deactivate)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::Deactivate)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::Activate)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::Activate)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::Activate)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::StartSeeking)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::StartSeeking)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::StartSeeking)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::StopTracking)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::StopTracking)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::StopTracking)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::PerspectiveRectInfo)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::PerspectiveRectInfo)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::PerspectiveRectInfo, data)


}