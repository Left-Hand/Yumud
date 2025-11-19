#pragma once

#include <cstdint>
#include "primitive/can/can_id.hpp"

namespace ymd::uavcan::primitive{


struct Header;

struct [[nodiscard]] NodeId{

    //  1：NodeID由7bit组成，其中0是保留ID，代表一个未知的节点。
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

    [[nodiscard]] constexpr uint8_t as_u7() const{
        return bits;
    }

    [[nodiscard]] constexpr uint8_t count() const{
        return as_u7();
    }
    [[nodiscard]] constexpr bool is_unknown() const {
        return count() == 0;
    }

    [[nodiscard]] constexpr bool is_preserved() const {
        return count() >= 126;
    }
};

struct [[nodiscard]] Priority{
    using Self = Priority;
    uint8_t count;

    static constexpr Option<Self> from_bits(const uint8_t bits){
        if(bits > 0b11111) return None;
        return Some(Self{bits});
    }
    [[nodiscard]] constexpr bool operator==(const Self & other) const = default;
    [[nodiscard]] constexpr bool is_senior_than(const Self & other) const {
        //less count means higher priority
        return count < other.count;
    }
};

struct [[nodiscard]] MessageFrameHeader final{
    using Self = MessageFrameHeader;

    static constexpr Self from_can_id(const hal::CanExtId & id){
        return std::bit_cast<Self>(id.to_u29());
    }

    [[nodiscard]] constexpr Priority priority() const {
        return Priority(priority_);
    }
    [[nodiscard]] constexpr uint32_t as_bits() const {
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr hal::CanExtId to_can_id() const {
        return hal::CanExtId(as_bits());
    }

    [[nodiscard]] NodeId source_id() const {
        return NodeId::from_bits(static_cast<uint8_t>(source_node_id_));
    }
private:
    uint32_t source_node_id_:7;
    const uint32_t __service_else_message__:1 = 0;
    uint32_t message_type_id_:16;
    uint32_t priority_:5;
    uint32_t :3;

    friend class Header;

    [[nodiscard]] static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }
};

struct [[nodiscard]] AnonymousFrameHeader final{
    using Self = AnonymousFrameHeader;   

    static constexpr Self from_can_id(const hal::CanExtId & id){
        return std::bit_cast<Self>(id.to_u29());
    }
    [[nodiscard]] constexpr Priority priority() const {
        return Priority(priority_);
    }
    [[nodiscard]] constexpr uint32_t as_bits() const {
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr hal::CanExtId to_can_id() const {
        return hal::CanExtId(as_bits());
    }

    [[nodiscard]] NodeId source_id() const {
        return NodeId::from_bits(static_cast<uint8_t>(source_node_id_));
    }
private:
    uint32_t source_node_id_:7;
    const uint32_t __service_else_message__:1 = 0;
    uint32_t message_type_id_:2;
    uint32_t discriminator:14;
    uint32_t priority_:5;
    uint32_t :3;
};

struct [[nodiscard]] ServiceFrameHeader final{
    using Self = ServiceFrameHeader;

    static constexpr Self from_can_id(const hal::CanExtId & id){
        return std::bit_cast<Self>(id.to_u29());
    }
    [[nodiscard]] constexpr Priority priority() const {
        return Priority(priority_);
    }
    [[nodiscard]] constexpr uint32_t as_bits() const {
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr hal::CanExtId to_can_id() const {
        return hal::CanExtId(as_bits());
    }

    constexpr NodeId dest_id() const {
        return NodeId::from_bits(static_cast<uint8_t>(dest_node_id_));
    }

    [[nodiscard]] constexpr bool is_request() const{
        return request_else_response_;
    }

    [[nodiscard]] NodeId source_id() const {
        return NodeId::from_bits(static_cast<uint8_t>(source_node_id_));
    }
private:
    uint32_t source_node_id_:7;
    const uint32_t __service_else_message__:1 = 1;
    uint32_t dest_node_id_:7;
    uint32_t request_else_response_:1;
    uint32_t service_type_id_:8;
    uint32_t priority_:5;
    uint32_t :3;

    friend class Header;

    [[nodiscard]] static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }
};

struct [[nodiscard]] TailByte{
    using Self = TailByte;
    uint8_t transfer_id:5;
    uint8_t toggle:1;
    uint8_t is_end_of_transfer:1;
    uint8_t is_start_of_transfer:1;

    static constexpr TailByte from_bits(uint8_t bits) {
        return std::bit_cast<TailByte>(bits);
    }
    [[nodiscard]] static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint8_t as_bits() const {
        return std::bit_cast<uint8_t>(*this);
    }
};

static_assert(sizeof(TailByte) == 1);


struct Header{
    using Self = Header;
    uint32_t bits;

    [[nodiscard]] static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint32_t as_bits() const {
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] static constexpr Self from_can_id(const hal::CanExtId & id){
        return Self::from_bits(id.to_u29());
    }

    template<typename T>
    requires (
        std::is_same_v<T, MessageFrameHeader> ||
        std::is_same_v<T, AnonymousFrameHeader> ||
        std::is_same_v<T, ServiceFrameHeader>
    )
    [[nodiscard]] constexpr T to_header() const {
        return T::from_bits(this->as_bits());
    }
    [[nodiscard]] constexpr hal::CanExtId to_can_id() const {
        return hal::CanExtId(this->as_bits());
    }

    [[nodiscard]] NodeId source_id() const {
        return NodeId::from_bits(static_cast<uint8_t>(as_bits() & 0b1111111));
    }
    [[nodiscard]] constexpr Priority priority() const {
        return Priority((bits >> 24) & 0b11111);
    }
};



};