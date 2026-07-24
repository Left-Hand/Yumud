#pragma once

#include <cstdint>
#include "primitive/can/can_id.hpp"
#include "core/string/view/string_view.hpp"

// https://blog.csdn.net/a_xiaoning/article/details/146126093
// https://github.com/elodin-sys/elodin/blob/1d1c42139e18fe6d5b425b64ca8b55b1d38741f2/fsw/sensor-fw/src/dronecan.rs#L2

namespace ymd::uavcan{

struct Header;


struct [[nodiscard]] SubjectId final{
    using Self = SubjectId;
    uint16_t bits;

    [[nodiscard]] static constexpr Self from_u16(const uint16_t b){
        return Self{b};
    }

    [[nodiscard]] constexpr bool is_standerd() const noexcept {
        return (bits < 20000);
    }

    [[nodiscard]] constexpr bool is_manufacturer_specific() const noexcept {
        return (bits >= 20000) and (bits < 21000);
    }

    [[nodiscard]] constexpr bool is_preserved() const noexcept {
        return bits >= 21000;
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) noexcept { 
        return os << self.bits;
    }
};

struct [[nodiscard]] ServiceId final{
    using Self = ServiceId;
    uint8_t bits;

    [[nodiscard]] constexpr Self from_u8(const uint8_t b){
        return Self{b};
    }

    [[nodiscard]] constexpr bool is_standerd() const noexcept {
        return (bits < 100);
    }

    [[nodiscard]] constexpr bool is_preserved() const noexcept {
        return (bits >= 100) and (bits < 200);
    }

    [[nodiscard]] constexpr bool is_manufacturer_specific() const noexcept {
        return bits >= 200;
    }
    friend OutputStream & operator<<(OutputStream & os, const Self & self) noexcept { 
        return os << self.bits;
    }
};

struct [[nodiscard]] NodeId final{

    // 1：NodeID由7bit组成，其中0是保留ID，代表一个未知的节点。
    // 2：Node ID取值为1-127，包含1-127，其中126，127是保留ID。
    // 3：NodeID分为SourceNodeID和DestinationNodeID。
    // 4：SourceNodeID表示节点自身的ID。
    // 5：DestinationNodeID表示对方的节点ID。
    // 6：只有Service帧才会有DestinationNodeID，需要应答。

    using Self = NodeId;

    uint8_t bits;

    static constexpr NodeId from_u7(const uint8_t bits){
        return NodeId{static_cast<uint8_t>(bits & 0b1111111)};
    }

    static constexpr NodeId from_bits(const uint8_t bits){
        return from_u7(bits);
    }

    [[nodiscard]] constexpr uint8_t to_u7() const noexcept {
        return bits;
    }

    [[nodiscard]] constexpr uint8_t count() const noexcept {
        return to_u7();
    }
    [[nodiscard]] constexpr bool is_unknown() const noexcept {
        return count() == 0;
    }

    [[nodiscard]] constexpr bool is_preserved() const noexcept {
        return count() >= 126;
    }
};

struct [[nodiscard]] Priority final{
    using Self = Priority;
    uint8_t count;

    static constexpr Self from_bits(const uint8_t bits){
        return Self{static_cast<uint8_t>(bits & 0b11111)};
    }

    static constexpr Option<Self> try_from_bits(const uint8_t bits){
        if(bits > 0b11111) return None;
        return Some(Self{bits});
    }

    [[nodiscard]] constexpr bool operator==(const Self & other) const = default;
    [[nodiscard]] constexpr bool is_senior_than(const Self & other) const noexcept {
        //less count means higher priority
        return count < other.count;
    }

    [[nodiscard]] constexpr bool is_high_priority() const noexcept {
        return count < 4;
    }

    friend OutputStream & operator<<(OutputStream & os, const Self & self) noexcept { 
        switch(self.count){
            case 0: return os << "Exceptional";
            case 1: return os << "Immediate";
            case 2: return os << "Fast";
            case 3: return os << "High";
            case 4: return os << "Nominal";
            case 5: return os << "Low";
            case 6: return os << "Slow";
            case 7: return os << "Optional";
        }
        return os << "Unknown";
    }
};

struct [[nodiscard]] MessageFrameHeader final{
    using Self = MessageFrameHeader;

    uint32_t bits;
    struct [[nodiscard]] BitFields final{
        uint32_t source_node_id:7;
        const uint32_t __service_else_message__:1;
        uint32_t subject_id:16;
        uint32_t priority:5;
        uint32_t :3;
    };


    static constexpr Self from_u32(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_can_id(const hal::CanExtId & id){
        return std::bit_cast<Self>(id.to_u29());
    }

    [[nodiscard]] constexpr uint32_t to_u32() const noexcept {
        return std::bit_cast<uint32_t>(*this);
    }

    constexpr hal::CanExtId to_can_id() const noexcept {
        return hal::CanExtId::from_bits(to_u32());
    }

    [[nodiscard]] constexpr Priority priority() const noexcept {
        return Priority::from_bits(std::bit_cast<BitFields>(bits).priority);
    }

    [[nodiscard]] NodeId source_node_id() const noexcept {
        return NodeId::from_u7(std::bit_cast<BitFields>(bits).source_node_id);
    }

    [[nodiscard]] SubjectId subject_id() const noexcept {
        return SubjectId::from_u16(std::bit_cast<BitFields>(bits).subject_id);
    }

    friend class Header;
};

struct [[nodiscard]] AnonymousMessageFrameHeader final{
    using Self = AnonymousMessageFrameHeader;   

    uint32_t bits;

    struct BitFields{
        uint32_t source_node_id:7;
        uint32_t __hwzero_service_else_message__:1;
        uint32_t message_type_id_:2;
        uint32_t discriminator:14;
        uint32_t priority:5;
        uint32_t :3;
    };

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_can_id(const hal::CanExtId & id){
        return std::bit_cast<Self>(id.to_u29());
    }

    [[nodiscard]] constexpr uint32_t to_bits() const noexcept {
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr hal::CanExtId to_can_id() const noexcept {
        return hal::CanExtId::from_bits(to_bits());
    }


    [[nodiscard]] constexpr Priority priority() const noexcept {
        return Priority(std::bit_cast<BitFields>(bits).priority);
    }

    [[nodiscard]] NodeId source_id() const noexcept {
        return NodeId::from_bits(static_cast<uint8_t>(std::bit_cast<BitFields>(bits).source_node_id));
    }


};

struct [[nodiscard]] ServiceFrameHeader final{
    using Self = ServiceFrameHeader;

    uint32_t bits;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    static constexpr Self from_can_id(const hal::CanExtId & id){
        return std::bit_cast<Self>(id.to_u29());
    }

    [[nodiscard]] constexpr uint32_t to_bits() const noexcept {
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr hal::CanExtId to_can_id() const noexcept {
        return hal::CanExtId::from_bits(to_bits());
    }

    [[nodiscard]] constexpr Priority priority() const noexcept {
        return Priority(std::bit_cast<BitFields>(bits).priority);
    }


    constexpr NodeId dest_id() const noexcept {
        return NodeId::from_bits(static_cast<uint8_t>(std::bit_cast<BitFields>(bits).dest_node_id_));
    }

    [[nodiscard]] constexpr bool is_request() const noexcept {
        return std::bit_cast<BitFields>(bits).request_else_response_;
    }

    [[nodiscard]] NodeId source_id() const noexcept {
        return NodeId::from_bits(static_cast<uint8_t>(std::bit_cast<BitFields>(bits).source_node_id));
    }
    struct BitFields{
        uint32_t source_node_id:7;
        uint32_t __hwone_service_else_message__:1;
        uint32_t dest_node_id_:7;
        uint32_t request_else_response_:1;
        uint32_t service_type_id_:8;
        uint32_t priority:5;
        uint32_t :3;
    };

    friend class Header;

};

struct [[nodiscard]] TailByte final{
    using Self = TailByte;

    uint8_t transfer_id:5;
    uint8_t toggle:1;
    uint8_t is_end_of_transfer:1;
    uint8_t is_start_of_transfer:1;

    [[nodiscard]] static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint8_t to_bits() const noexcept {
        return std::bit_cast<uint8_t>(*this);
    }
};

static_assert(sizeof(TailByte) == 1);

struct [[nodiscard]] Header final{
    using Self = Header;
    uint32_t bits;

    [[nodiscard]] static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint32_t to_bits() const noexcept {
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] static constexpr Self from_can_id(const hal::CanExtId & id){
        return Self::from_bits(id.to_u29());
    }

    template<typename T>
    requires (
        std::is_same_v<T, MessageFrameHeader> ||
        std::is_same_v<T, AnonymousMessageFrameHeader> ||
        std::is_same_v<T, ServiceFrameHeader>
    )
    [[nodiscard]] constexpr T to_header() const noexcept {
        return T::from_bits(this->to_bits());
    }

    [[nodiscard]] constexpr hal::CanExtId to_can_id() const noexcept {
        return hal::CanExtId::from_bits(this->to_bits());
    }

    [[nodiscard]] constexpr NodeId source_id() const noexcept {
        return NodeId::from_bits(static_cast<uint8_t>(to_bits() & 0b1111111));
    }

    [[nodiscard]] constexpr bool is_service_frame() const noexcept {
        return bits & (1u << 7);
    }

    [[nodiscard]] constexpr bool is_message_frame() const noexcept {
        return !is_service_frame();
    }

    [[nodiscard]] constexpr bool is_anonymous_message_frame() const noexcept {
        return (bits & 0xff) == 0;
    }

    [[nodiscard]] constexpr Priority priority() const noexcept {
        return Priority((bits >> 24) & 0b11111);
    }
};

}