#pragma once

#include "primitive/can/can_id.hpp"

namespace ymd::cyphal{


struct [[nodiscard]] NodeId{
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
};

enum class [[nodiscard]] TransferPriority:uint8_t{
    Exceptional,
    Immediate,
    Fast,
    High,
    Normnial,
    Low,
    Slow,
    Optional,
    Default = Normnial
};

enum class [[nodiscard]] TransferKind:uint8_t{
    Message,
    Service
};


[[nodiscard]] static constexpr uint8_t compute_cyclic_transfer_id_difference(
    const uint8_t a,
    const uint8_t b,
    const uint8_t modulo
){
    const int16_t d = static_cast<int16_t>(a) - static_cast<int16_t>(b);
    if(d < 0) return static_cast<uint8_t>(d + modulo);
    return static_cast<uint8_t>(d);
}

struct [[nodiscard]] MessageHeader{
public:
    using Self = MessageHeader;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint32_t as_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] constexpr TransferPriority priority() const{
        return std::bit_cast<TransferPriority>(static_cast<uint8_t>(priority_));
    }

    [[nodiscard]] constexpr NodeId source_id() const{
        return NodeId::from_bits(source_node_id_);
    }

    [[nodiscard]] constexpr bool is_anoymous() const{
        return anonymous_ == 0;
    }

    [[nodiscard]] constexpr uint16_t subject_id() const{
        return subject_id_;
    }

private:
    uint32_t source_node_id_:7;
    const uint32_t __hw_0:1 = 0;
    uint32_t subject_id_:13;
    const uint32_t __hw_1:3 = 0b011;
    uint32_t anonymous_:1;
    const uint32_t __hw_2:1 = 0b0;
    uint32_t priority_:3;
    uint32_t __resv__:3;
};


struct [[nodiscard]] ServiceHeader{
public:
    using Self = ServiceHeader;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint32_t as_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] constexpr TransferPriority priority() const{
        return static_cast<TransferPriority>(priority_);
    }

    [[nodiscard]] constexpr NodeId source_id() const{
        return NodeId::from_bits(source_node_id_);
    }

    [[nodiscard]] constexpr NodeId destination_id() const{
        return NodeId::from_bits(destination_node_id_);
    }

    [[nodiscard]] constexpr bool is_request() const{
        return is_request_;
    }

    [[nodiscard]] constexpr uint16_t service_id() const{
        return service_id_;
    }
private:
    uint32_t source_node_id_:7;
    uint32_t destination_node_id_:7;
    uint32_t service_id_:9;
    const uint32_t __hw_0:1 = 0;
    uint32_t is_request_:1;
    const uint32_t __hw_1:1 = 1;
    uint32_t priority_:3;
    uint32_t __resv__:3;
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



}