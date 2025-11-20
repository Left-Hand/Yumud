#pragma once

#include "../canopen_primitive/canopen_nmt_primitive.hpp"
#include "canopen_msg_serde.hpp"


namespace ymd::canopen::nmt_msg{
using namespace canopen::primitive;


struct [[nodiscard]] NetManage{
    //网络管理报文 
    using Self = NetManage;
    static constexpr auto COBID = CobId::from_bits(0x00);

    NmtCommand cmd;
    NodeId dest_nodeid;

    [[nodiscard]] std::span<const uint8_t, 2> as_le_bytes() const{
        return std::span<const uint8_t, 2>(reinterpret_cast<const uint8_t*>(this), 2);
    }

    [[nodiscard]] std::span<uint8_t, 2> as_mut_le_bytes(){
        return std::span<uint8_t, 2>(reinterpret_cast<uint8_t*>(this), 2);
    }

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = std::bit_cast<uint8_t>(cmd);
        bytes[1] = dest_nodeid.to_u7();
    }



};

struct [[nodiscard]] Sync{
    // 同步功能用于让总线上所有节点同步，主要用于让节点同步 PDO 消息。其 COB-ID 固定为
    // 0x80，数据域为空。
    static constexpr auto COBID = CobId::from_bits(0x80);
};


// BootUp已经弃用，被HeartBeat复用
// struct [[nodiscard]] BootUp{ 
//     using Self = BootUp;
//     NodeId station_nodeid;
// };


struct [[nodiscard]] Heartbeat{ 
    using Self = Heartbeat;
    NodeId station_nodeid;
    NodeState station_state;
    [[nodiscard]] static constexpr Self from_bootup(const NodeId station_nodeid){
        return Self{
            .station_nodeid = station_nodeid,
            .station_state = NodeState::BootUp
        };
    }
    [[nodiscard]] bool is_bootup() const {
        return station_state == NodeState::BootUp;
    }
};

}

namespace ymd::canopen::msg_serde{


template<>
struct MsgSerde<nmt_msg::NetManage>{
    using Self = nmt_msg::NetManage;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        std::array<uint8_t, 2> bytes;
        self.fill_bytes(bytes);
        return CanMsg::from_bytes(Self::COBID.to_stdid(), std::span(bytes));
    }

    template<AssertLevel assert_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 0);

        const auto canid_u32 = msg.id_u32();

        FLEX_EXTERNAL_ASSERT_NONE(canid_u32 == 0);

        const auto self = Self{
            .cmd = FLEX_TRY_UNWRAP_RESULT_TO_OPTION(convert::try_cast<NmtCommand>(msg[0])),
            .dest_nodeid = NodeId::from_u7(msg[1]),
        };
        FLEX_RETURN_SOME(self);
    }
};


template<>
struct MsgSerde<nmt_msg::Sync>{
    using Self = nmt_msg::Sync;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        return CanMsg::from_empty(Self::COBID.to_stdid());
    }

    template<AssertLevel assert_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 0);

        const auto canid_u32 = msg.id_u32();

        FLEX_EXTERNAL_ASSERT_NONE(canid_u32 == Self::COBID.to_bits());

        const auto self = Self{};
        FLEX_RETURN_SOME(self);
    }
};


template<>
struct MsgSerde<nmt_msg::Heartbeat>{
    using Self = nmt_msg::Heartbeat;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self & self){
        const auto canid = hal::CanStdId::from_bits(0x700 | self.station_nodeid.to_u7());
        const std::array<uint8_t, 1> bytes = {static_cast<uint8_t>(self.station_state)};
        return CanMsg::from_bytes(canid, std::span(bytes));
    }

    template<AssertLevel assert_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
    -> FLEX_OPTION(Self){
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 1);

        const auto canid_u32 = msg.id_u32();

        FLEX_EXTERNAL_ASSERT_NONE((canid_u32 & 0b111'1000'0000) == 0x700);
        const auto self = Self{
            .station_nodeid = NodeId::from_u7(canid_u32 & 0x7f),
            .station_state = std::bit_cast<NodeState>(msg[0])
        };
        FLEX_RETURN_SOME(self);
    }
};



}