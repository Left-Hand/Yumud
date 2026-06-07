#pragma once

#include "../canopen_primitive/canopen_nmt_primitive.hpp"
#include "canopen_msg_serde.hpp"
#include "core/utils/bits/bits_caster.hpp"


namespace ymd::canopen::nmt_msgs{
using namespace canopen::primitive;

// NMT（Network Management）报文
// 功能：网络管理，用于启动、停止、复位节点等操作
// COB-ID：0x000（固定）
// 包括：
// NMT命令消息（主站发送）
// 节点状态变化通知（从站发送）
struct [[nodiscard]] NetManage final{
    //网络管理报文 
    using Self = NetManage;
    static constexpr auto COBID = CobId::from_bits(0x00);

    NmtCommand cmd;
    NodeId dest_node_id;

    constexpr void fill_payload_bytes(const std::span<uint8_t, 2> bytes) const noexcept {
        bytes[0] = std::bit_cast<uint8_t>(cmd);
        bytes[1] = dest_node_id.to_u7();
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame & frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 2);

        const auto canid_u32 = frame.id_u32();

        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(canid_u32 == 0);

        const auto self = Self{
            .cmd = CANOPEN_FLEX_TRY_UNWRAP_RESULT_TO_OPTION(convert::try_cast<NmtCommand>(frame[0])),
            .dest_node_id = NodeId::from_u7(frame[1]),
        };
        CANOPEN_RETURN_SOME(self);
    }
};

// SYNC 报文
// 功能：同步信号，用于同步所有节点的操作
// COB-ID：0x080（固定）
// 特点：无数据域，仅作为同步触发信号
struct [[nodiscard]] Sync final{
    using Self = Sync;

    // 同步功能用于让总线上所有节点同步，主要用于让节点同步 PDO 消息。其 COB-ID 固定为
    // 0x80，数据域为空。
    static constexpr auto COBID = CobId::from_bits(0x80);

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        return CanFrame::from_empty_data(COBID.to_stdid());
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame & frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 0);

        const auto canid_u32 = frame.id_u32();

        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(canid_u32 == Self::COBID.to_bits());

        const auto self = Self{};
        CANOPEN_RETURN_SOME(self);
    }
};

// Emergency报文
// 功能：紧急错误报告
// COB-ID：0x080 + NodeID
// 数据：错误代码、错误寄存器、厂商特定错误字段等
struct [[nodiscard]] Emergency final{
    using Self = Emergency;


    NodeId station_node_id;
    EmcyErrorCode error_code;
    uint8_t error_register;
    std::array<uint8_t, 5> manufacturer_specific;

    [[nodiscard]] constexpr CobId cobid() const noexcept {
        return CobId::from_bits(0x080 | station_node_id.to_u7());
    }

    constexpr void fill_payload_bytes(const std::span<uint8_t, 8> bytes) const noexcept {
        bytes[0] = static_cast<uint8_t>(error_code.to_bits() >> 8);
        bytes[1] = static_cast<uint8_t>(error_code.to_bits());
        bytes[2] = error_register;
        bytes[3] = manufacturer_specific[0];
        bytes[4] = manufacturer_specific[1];
        bytes[5] = manufacturer_specific[2];
        bytes[6] = manufacturer_specific[3];
        bytes[7] = manufacturer_specific[4];
    }

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        auto & self = *this;
        const auto can_id = self.cobid().to_stdid();
        std::array<uint8_t, 8> u8x8;
        self.fill_payload_bytes(u8x8);
        return CanFrame::from_parts(can_id, CanPayload::from_u8x8(u8x8));
    }


    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame & frame)
        -> CANOPEN_FLEX_OPTION(Self) {
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 8);

        const auto canid_u32 = frame.id_u32();
        const auto bytes = frame.payload_bytes_sized<8>();

        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE((canid_u32 & 0b11110000000) == 0x080);
        
        const uint16_t error_code_u16 = (bytes[0] << 8) | bytes[1];
        const auto self = Self{
            .station_node_id = NodeId::from_u7(canid_u32 & 0x7f),
            .error_code = CANOPEN_FLEX_TRY_UNWRAP_RESULT_TO_OPTION(
                convert::try_cast<EmcyErrorCode>(error_code_u16)),
            .error_register = bytes[2],
            .manufacturer_specific = {bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]}
        };
        CANOPEN_RETURN_SOME(self);
    }
};


// BootUp已经弃用，被HeartBeat复用
// struct [[nodiscard]] BootUp{ 
//     using Self = BootUp;
//     NodeId station_node_id;
// };

struct [[nodiscard]] NodeGuardingRequest final {
    // Node guarding request message
    using Self = NodeGuardingRequest;
    NodeId target_node_id;
    
    [[nodiscard]] constexpr CobId cobid() const noexcept {
        return CobId::from_bits(0x700 | target_node_id.to_u7());
    }

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        auto & self = *this;
        return CanFrame::from_empty_data(self.cobid().to_stdid());
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame & frame)
        -> CANOPEN_FLEX_OPTION(Self) {
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 0);

        const auto canid_u32 = frame.id_u32();
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE((canid_u32 & 0b11110000000) == 0x700);
        
        const auto self = Self{
            .target_node_id = NodeId::from_u7(canid_u32 & 0x7f)
        };
        CANOPEN_RETURN_SOME(self);
    }
};

struct [[nodiscard]] NodeGuardingResponse final {
    // Node guarding response message
    using Self = NodeGuardingResponse;
    NodeId station_node_id;
    NodeState station_state;
    
    [[nodiscard]] constexpr CobId cobid() const noexcept {
        return CobId::from_bits(0x700 | station_node_id.to_u7());
    }

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        auto & self = *this;
        const auto can_id = self.cobid().to_stdid();
        const std::array<uint8_t, 1> bytes = {static_cast<uint8_t>(self.station_state)};
        return CanFrame::from_parts(can_id, CanPayload::from_bytes(std::span(bytes)));
    }

    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame & frame)
        -> CANOPEN_FLEX_OPTION(Self) {
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 1);

        const auto canid_u32 = frame.id_u32();
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE((canid_u32 & 0b11110000000) == 0x700);
        
        const auto self = Self{
            .station_node_id = NodeId::from_u7(canid_u32 & 0x7f),
            .station_state = std::bit_cast<NodeState>(frame[0])
        };
        CANOPEN_RETURN_SOME(self);
    }

};

struct [[nodiscard]] Heartbeat final{  
    using Self = Heartbeat;
    NodeId station_node_id;
    NodeState station_state;


    [[nodiscard]] static constexpr Self from_bootup(const NodeId station_node_id){
        return Self{
            .station_node_id = station_node_id,
            .station_state = NodeState::BootUp
        };
    }

    [[nodiscard]] constexpr CobId cobid() const noexcept {
        return CobId::from_bits(0x700 | station_node_id.to_u7());
    }

    [[nodiscard]] bool is_bootup() const noexcept {
        return station_state == NodeState::BootUp;
    }

    [[nodiscard]] constexpr CanFrame to_can_frame() const noexcept {
        auto & self = *this;
        const auto can_id = self.cobid().to_stdid();
        const std::array<uint8_t, 1> bytes = {static_cast<uint8_t>(self.station_state)};
        return CanFrame::from_parts(can_id, CanPayload::from_bytes(std::span(bytes)));
    }


    template<VerifyLevel VERIFY_LEVEL>
    [[nodiscard]] static constexpr auto from_can_frame(const CanFrame & frame)
    -> CANOPEN_FLEX_OPTION(Self){
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.is_standard());
        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE(frame.length() == 1);

        const auto canid_u32 = frame.id_u32();

        CANOPEN_EXTERNAL_RETURN_NONE_IFFALSE((canid_u32 & 0b111'1000'0000) == 0x700);
        const auto self = Self{
            .station_node_id = NodeId::from_u7(canid_u32 & 0x7f),
            .station_state = std::bit_cast<NodeState>(frame[0])
        };
        CANOPEN_RETURN_SOME(self);
    }
};

}