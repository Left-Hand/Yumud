#pragma once

#include "core/utils/serde.hpp"

namespace ymd{


namespace robots::joint_commands{

struct [[nodiscard]] SetPosition{
    math::bf16 position;
};


struct [[nodiscard]] DeltaPosition{
    iq20 delta_position;
};


struct [[nodiscard]] SetSpeed{
    math::bf16 speed;
};


struct [[nodiscard]] SetPositionWithFwdSpeed{
    math::bf16 position;
    math::bf16 speed;
};


struct [[nodiscard]] SetTrapzoid{
    math::bf16 position;
    math::bf16 speed;
    math::bf16 acceleration;
    math::bf16 deceleration;
};


struct [[nodiscard]] SetKpKd{
    math::bf16 kp;
    math::bf16 kd;
};


struct [[nodiscard]] SetKpKiKd{
    math::bf16 kp;
    math::bf16 ki;
    math::bf16 kd;
};

struct [[nodiscard]] Deactivate{

};

struct [[nodiscard]] Activate{

};

struct [[nodiscard]] StartSeeking{

};

struct [[nodiscard]] StopUpdating{

};

struct [[nodiscard]] StopTracking{

};


struct [[nodiscard]] StartTracking{

};


struct [[nodiscard]] PerspectiveRectInfo{
    // std::array<Vec2<uint8_t>, 4> points;
    std::array<uint8_t, 8> data;
};


struct [[nodiscard]] ErrXY{
    math::bf16 px;
    math::bf16 py;
};

struct [[nodiscard]] FwdXY{
    math::bf16 fx;
    math::bf16 fy;
};


}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetPosition)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetPosition)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::SetPosition, position)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::SetSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::SetSpeed)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::SetSpeed, speed)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::DeltaPosition)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::DeltaPosition)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::DeltaPosition, delta_position)

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

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::StopUpdating)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::StopUpdating)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::StopUpdating)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::StartSeeking)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::StartSeeking)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::StartSeeking)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::StopTracking)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::StopTracking)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::StopTracking)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::StartTracking)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::StartTracking)
DEF_DERIVE_MEM_REFLECTER_0(robots::joint_commands::StartTracking)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::PerspectiveRectInfo)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::PerspectiveRectInfo)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_commands::PerspectiveRectInfo, data)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::ErrXY)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::ErrXY)
DEF_DERIVE_MEM_REFLECTER_2(robots::joint_commands::ErrXY, px, py)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_commands::FwdXY)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_commands::FwdXY)
DEF_DERIVE_MEM_REFLECTER_2(robots::joint_commands::FwdXY, fx, fy)


}