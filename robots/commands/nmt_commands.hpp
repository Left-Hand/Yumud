#pragma once

#include "core/utils/serde.hpp"

namespace ymd{
namespace robots::nmt_commands{

enum class NodeState:uint8_t{
    Reset = 0x00,
    Initialization = 0x01,
    PreOperational = 0x02,
    Stopped = 0x03
};

struct ResetNode{
    uint16_t timeout;
};

struct PreOperationalNode{
};

struct StartNode{
};

struct StopNode{
};

struct BroadcastBootUp{
};

struct BroadcastHeartBeat{
    uint16_t since_last_heartbeat_ms;
};

struct SetHeartBeatDuration{
    uint16_t target_duration_ms;
};

struct RequestNodeState{

};

struct ResponseNodeState{
    NodeState node_state;
};

}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::ResetNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::ResetNode)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_commands::ResetNode, timeout)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::PreOperationalNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::PreOperationalNode)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_commands::PreOperationalNode)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::StartNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::StartNode)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_commands::StartNode)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::StopNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::StopNode)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_commands::StopNode)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::BroadcastBootUp)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::BroadcastBootUp)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_commands::BroadcastBootUp)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::BroadcastHeartBeat)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::BroadcastHeartBeat)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_commands::BroadcastHeartBeat, since_last_heartbeat_ms)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::SetHeartBeatDuration)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::SetHeartBeatDuration)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_commands::SetHeartBeatDuration, target_duration_ms)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::RequestNodeState)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::RequestNodeState)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_commands::RequestNodeState)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_commands::ResponseNodeState)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_commands::ResponseNodeState)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_commands::ResponseNodeState, node_state)
}