#include "canopen_nmt_msg.hpp"

using namespace ymd::canopen;
using namespace ymd::canopen::primitive;
using namespace ymd::canopen::msg_serde;
using namespace ymd::canopen::nmt_msg;

[[maybe_unused]] static void canopen_static_test2(){
    static constexpr auto canmsg = MsgSerde<BootUp>::to_canmsg(BootUp{.station_nodeid = NodeId::from_u7(5)});
    static_assert(canmsg.is_standard());
    static_assert(canmsg.length() == 1);
    static_assert(canmsg.payload_bytes()[0] == 0);
    static_assert(msg_serde::from_canmsg<BootUp, AssertLevel::NoCheck>(canmsg)
        .station_nodeid == NodeId::from_u7(5));
    static_assert(msg_serde::from_canmsg<BootUp, AssertLevel::Propagate>(canmsg).unwrap()
        .station_nodeid == NodeId::from_u7(5));
}