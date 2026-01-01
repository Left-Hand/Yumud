#pragma once

#include "core/utils/serde.hpp"

namespace ymd{
namespace robots::nmt_msgs{

enum class [[nodiscard]] NodeState:uint8_t{
    Reset = 0x00,
    Initialization = 0x01,
    PreOperational = 0x02,
    Stopped = 0x03
};

struct [[nodiscard]] ResetNode{
    uint16_t timeout;
};

struct [[nodiscard]] PreOperationalNode{
};

struct [[nodiscard]] StartNode{
};

struct [[nodiscard]] StopNode{
};

struct [[nodiscard]] BroadcastBootUp{
};

struct [[nodiscard]] BroadcastHeartBeat{
    uint16_t since_last_heartbeat_ms;
};

struct [[nodiscard]] SetHeartBeatDuration{
    uint16_t target_duration_ms;
};

struct [[nodiscard]] RequestNodeState{

};

struct [[nodiscard]] ResponseNodeState{
    NodeState node_state;
};

}

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::ResetNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::ResetNode)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_msgs::ResetNode, timeout)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::PreOperationalNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::PreOperationalNode)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_msgs::PreOperationalNode)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::StartNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::StartNode)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_msgs::StartNode)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::StopNode)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::StopNode)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_msgs::StopNode)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::BroadcastBootUp)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::BroadcastBootUp)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_msgs::BroadcastBootUp)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::BroadcastHeartBeat)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::BroadcastHeartBeat)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_msgs::BroadcastHeartBeat, since_last_heartbeat_ms)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::SetHeartBeatDuration)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::SetHeartBeatDuration)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_msgs::SetHeartBeatDuration, target_duration_ms)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::RequestNodeState)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::RequestNodeState)
DEF_DERIVE_MEM_REFLECTER_0(robots::nmt_msgs::RequestNodeState)

DEF_DERIVE_SERIALIZE_AS_TUPLE(robots::nmt_msgs::ResponseNodeState)
DEF_DERIVE_RAW_BYTES_DESERIALIZER(robots::nmt_msgs::ResponseNodeState)
DEF_DERIVE_MEM_REFLECTER_1(robots::nmt_msgs::ResponseNodeState, node_state)
}