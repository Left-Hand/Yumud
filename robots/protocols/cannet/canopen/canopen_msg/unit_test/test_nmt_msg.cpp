#include "../canopen_nmt_msg.hpp"

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

[[maybe_unused]] static void canopen_node_guarding_request_static_test() {
    using Msg = NodeGuardingRequest;
    static constexpr auto msg = Msg{
        .target_nodeid = NodeId::from_u7(3)
    };
    static constexpr auto canmsg = msg_serde::to_canmsg(msg);
    static_assert(canmsg.is_standard());
    static_assert(canmsg.length() == 0);
    static_assert(canmsg.id_u32() == 0x703); // 0x700 | 3
    
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .target_nodeid == NodeId::from_u7(3));
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::Propagate>(canmsg).unwrap()
        .target_nodeid == NodeId::from_u7(3));
}

[[maybe_unused]] static void canopen_node_guarding_response_static_test() {
    using Msg = NodeGuardingResponse;
    static constexpr auto msg = Msg{
        .station_nodeid = NodeId::from_u7(7),
        .station_state = NodeState::PreOperational
    };
    static constexpr auto canmsg = msg_serde::to_canmsg(msg);
    static_assert(canmsg.is_standard());
    static_assert(canmsg.length() == 1);
    static_assert(canmsg.id_u32() == 0x707); // 0x700 | 7
    static_assert(canmsg.payload_bytes()[0] == static_cast<uint8_t>(NodeState::PreOperational));
    
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .station_nodeid == NodeId::from_u7(7));
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .station_state == NodeState::PreOperational);
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::Propagate>(canmsg).unwrap()
        .station_nodeid == NodeId::from_u7(7));
}

[[maybe_unused]] static void canopen_emergency_static_test() {
    using Msg = Emergency;
    static constexpr auto msg = Msg{
        .station_nodeid = NodeId::from_u7(5),
        .error_code = EmcyErrorCode::Communication,
        .error_register = 0x08,
        .manufacturer_specific = {0x11, 0x22, 0x33, 0x44, 0x55}
    };
    
    static constexpr auto canmsg = msg_serde::to_canmsg(msg);
    static_assert(canmsg.is_standard());
    static_assert(canmsg.length() == 8);
    static_assert(canmsg.id_u32() == 0x085); // 0x080 | 5
    static_assert(canmsg.payload_bytes()[0] == 0x81); // High byte of error_code
    static_assert(canmsg.payload_bytes()[1] == 0x00); // Low byte of error_code
    static_assert(canmsg.payload_bytes()[2] == 0x08); // error_register
    static_assert(canmsg.payload_bytes()[3] == 0x11); // manufacturer_specific[0]
    static_assert(canmsg.payload_bytes()[4] == 0x22); // manufacturer_specific[1]
    static_assert(canmsg.payload_bytes()[5] == 0x33); // manufacturer_specific[2]
    static_assert(canmsg.payload_bytes()[6] == 0x44); // manufacturer_specific[3]
    static_assert(canmsg.payload_bytes()[7] == 0x55); // manufacturer_specific[4]
    
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .station_nodeid == NodeId::from_u7(5));
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .error_code == EmcyErrorCode::Communication);
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::NoCheck>(canmsg)
        .error_register == 0x08);
    static_assert(msg_serde::from_canmsg<Msg, AssertLevel::Propagate>(canmsg).unwrap()
        .station_nodeid == NodeId::from_u7(5));
}