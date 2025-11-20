#include "canopen_nmt_msg.hpp"

using namespace ymd;
using namespace ymd::canopen;
using namespace ymd::canopen::primitive;
using namespace ymd::canopen::msg_serde;
using namespace ymd::canopen::nmt_msg;

[[maybe_unused]] static void canopen_msg_static_test1(){
    using Msg = Heartbeat;
    static constexpr auto msg = Msg{
        .station_nodeid = NodeId::from_u7(5),
        .station_state = NodeState::Operating
    };
    static constexpr auto canmsg = msg_serde::to_canmsg(msg);
    static_assert(canmsg.is_standard());
    static_assert(canmsg.length() == 1);
    static_assert(canmsg.payload_bytes()[0] == static_cast<uint8_t>(NodeState::Operating));
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .station_nodeid == NodeId::from_u7(5));
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .station_state == NodeState::Operating);
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::Propagate>(canmsg).unwrap()
        .station_nodeid == NodeId::from_u7(5));
}