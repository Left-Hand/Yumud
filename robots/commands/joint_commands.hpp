#pragma once

#include "core/utils/serde.hpp"

namespace ymd{


namespace robots::joint_cmds{

struct SetPosition{
    bf16 position;
};


struct SetSpeed{
    bf16 speed;
};


struct SetPositionAndSpeed{
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

}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetPosition)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetPosition)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_cmds::SetPosition, position)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetSpeed)
DEF_DERIVE_MEM_REFLECTER_1(robots::joint_cmds::SetSpeed, speed)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetPositionAndSpeed)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetPositionAndSpeed)
DEF_DERIVE_MEM_REFLECTER_2(robots::joint_cmds::SetPositionAndSpeed, position, speed)


DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetTrapzoid)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetTrapzoid)
DEF_DERIVE_MEM_REFLECTER_4(robots::joint_cmds::SetTrapzoid, position, speed, acceleration, deceleration)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetKpKd)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetKpKd)
DEF_DERIVE_MEM_REFLECTER_2(robots::joint_cmds::SetKpKd, kp, kd)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::joint_cmds::SetKpKiKd)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::joint_cmds::SetKpKiKd)
DEF_DERIVE_MEM_REFLECTER_3(robots::joint_cmds::SetKpKiKd, kp, ki, kd)


}