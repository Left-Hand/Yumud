#pragma once

#include "../canopen_primitive/canopen_nmt_primitive.hpp"
#include "canopen_msg_serde.hpp"
#include "core/utils/bits/bits_caster.hpp"


namespace ymd::canopen::nmt_msg{
using namespace canopen::primitive;

// NMT（Network Management）报文
// 功能：网络管理，用于启动、停止、复位节点等操作
// COB-ID：0x000（固定）
// 包括：
// NMT命令消息（主站发送）
// 节点状态变化通知（从站发送）
struct [[nodiscard]] NetManage{
    //网络管理报文 
    using Self = NetManage;
    static constexpr auto COBID = CobId::from_bits(0x00);

    NmtCommand cmd;
    NodeId dest_nodeid;

    constexpr void fill_bytes(const std::span<uint8_t, 2> bytes) const {
        bytes[0] = std::bit_cast<uint8_t>(cmd);
        bytes[1] = dest_nodeid.to_u7();
    }
};

// SYNC 报文
// 功能：同步信号，用于同步所有节点的操作
// COB-ID：0x080（固定）
// 特点：无数据域，仅作为同步触发信号
struct [[nodiscard]] Sync{
    // 同步功能用于让总线上所有节点同步，主要用于让节点同步 PDO 消息。其 COB-ID 固定为
    // 0x80，数据域为空。
    static constexpr auto COBID = CobId::from_bits(0x80);
};

// Emergency报文
// 功能：紧急错误报告
// COB-ID：0x080 + NodeID
// 数据：错误代码、错误寄存器、厂商特定错误字段等
struct [[nodiscard]] Emergency {
    using Self = Emergency;
    NodeId station_nodeid;
    EmcyErrorCode error_code;
    uint8_t error_register;
    std::array<uint8_t, 5> manufacturer_specific;

    [[nodiscard]] constexpr CobId cobid() const {
        return CobId::from_bits(0x080 | station_nodeid.to_u7());
    }

    constexpr void fill_bytes(const std::span<uint8_t, 8> bytes) const {
        bytes[0] = static_cast<uint8_t>(error_code.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(error_code.to_bits());
        bytes[2] = error_register;
        bytes[3] = manufacturer_specific[0];
        bytes[4] = manufacturer_specific[1];
        bytes[5] = manufacturer_specific[2];
        bytes[6] = manufacturer_specific[3];
        bytes[7] = manufacturer_specific[4];
    }
};


// BootUp已经弃用，被HeartBeat复用
// struct [[nodiscard]] BootUp{ 
//     using Self = BootUp;
//     NodeId station_nodeid;
// };

struct [[nodiscard]] NodeGuardingRequest {
    // Node guarding request message
    using Self = NodeGuardingRequest;
    NodeId target_nodeid;
    
    [[nodiscard]] constexpr CobId cobid() const {
        return CobId::from_bits(0x700 | target_nodeid.to_u7());
    }
};

struct [[nodiscard]] NodeGuardingResponse {
    // Node guarding response message
    using Self = NodeGuardingResponse;
    NodeId station_nodeid;
    NodeState station_state;
    
    [[nodiscard]] constexpr CobId cobid() const {
        return CobId::from_bits(0x700 | station_nodeid.to_u7());
    }
};

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
        return CanMsg(
            Self::COBID.to_stdid(), 
            hal::CanClassicPayload::from_bytes(std::span(bytes))
        );
    }

    template<VerifyLevel verify_level>
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

    template<VerifyLevel verify_level>
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
        return CanMsg(canid, CanPayload::from_bytes(std::span(bytes)));
    }

    template<VerifyLevel verify_level>
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


template<>
struct MsgSerde<nmt_msg::NodeGuardingRequest> {
    using Self = nmt_msg::NodeGuardingRequest;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self& self) {
        return CanMsg::from_empty(self.cobid().to_stdid());
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
        -> FLEX_OPTION(Self) {
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 0);

        const auto canid_u32 = msg.id_u32();
        FLEX_EXTERNAL_ASSERT_NONE((canid_u32 & 0b11110000000) == 0x700);
        
        const auto self = Self{
            .target_nodeid = NodeId::from_u7(canid_u32 & 0x7f)
        };
        FLEX_RETURN_SOME(self);
    }
};

template<>
struct MsgSerde<nmt_msg::NodeGuardingResponse> {
    using Self = nmt_msg::NodeGuardingResponse;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self& self) {
        const auto canid = self.cobid().to_stdid();
        const std::array<uint8_t, 1> bytes = {static_cast<uint8_t>(self.station_state)};
        return CanMsg(canid, CanPayload::from_bytes(std::span(bytes)));
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
        -> FLEX_OPTION(Self) {
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 1);

        const auto canid_u32 = msg.id_u32();
        FLEX_EXTERNAL_ASSERT_NONE((canid_u32 & 0b11110000000) == 0x700);
        
        const auto self = Self{
            .station_nodeid = NodeId::from_u7(canid_u32 & 0x7f),
            .station_state = std::bit_cast<NodeState>(msg[0])
        };
        FLEX_RETURN_SOME(self);
    }
};

template<>
struct MsgSerde<nmt_msg::Emergency> {
    using Self = nmt_msg::Emergency;
    [[nodiscard]] static constexpr CanMsg to_canmsg(const Self& self) {
        const auto canid = self.cobid().to_stdid();
        std::array<uint8_t, 8> bytes{};
        self.fill_bytes(bytes);
        return CanMsg(canid, CanPayload::from_bytes(std::span(bytes)));
    }

    template<VerifyLevel verify_level>
    [[nodiscard]] static constexpr auto from_canmsg(const CanMsg& msg)
        -> FLEX_OPTION(Self) {
        FLEX_EXTERNAL_ASSERT_NONE(msg.is_standard());
        FLEX_EXTERNAL_ASSERT_NONE(msg.length() == 8);

        const auto canid_u32 = msg.id_u32();
        FLEX_EXTERNAL_ASSERT_NONE((canid_u32 & 0b11110000000) == 0x080);
        
        const uint16_t error_code_u16 = (msg[0] << 8) | msg[1];
        const auto self = Self{
            .station_nodeid = NodeId::from_u7(canid_u32 & 0x7f),
            .error_code = FLEX_TRY_UNWRAP_RESULT_TO_OPTION(
                convert::try_cast<EmcyErrorCode>(error_code_u16)),
            .error_register = msg[2],
            .manufacturer_specific = {msg[3], msg[4], msg[5], msg[6], msg[7]}
        };
        FLEX_RETURN_SOME(self);
    }
};
}