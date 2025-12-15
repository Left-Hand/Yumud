#pragma once

#include "../cyphal_primitive.hpp"

namespace ymd::cyphal::transport::can{

using namespace cyphal::primitive;

struct [[nodiscard]] MessageHeader{
public:
    using Self = MessageHeader;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint32_t to_bits() const{
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
    const uint32_t __hw_wired_0__:1 = 0;
    uint32_t subject_id_:13;
    const uint32_t __hw_wired_1__:3 = 0b011;
    uint32_t anonymous_:1;
    const uint32_t __hw_wired_2__:1 = 0b0;
    uint32_t priority_:3;
    uint32_t __resv__:3;
};


struct [[nodiscard]] ServiceHeader{
public:
    using Self = ServiceHeader;

    static constexpr Self from_bits(const uint32_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint32_t to_bits() const{
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
    const uint32_t __hw_wired_0__:1 = 0;
    uint32_t is_request_:1;
    const uint32_t __hw_wired_1__:1 = 1;
    uint32_t priority_:3;
    uint32_t __resv__:3;
};


struct [[nodiscard]] TailByte{
    using Self = TailByte;
    uint8_t transfer_id:5;

    //4.2.2.2 ToggleBit
    // Transport frames that form a multi-frame transfer are equipped with a toggle bit which alternates its state
    // every frame within the transfer for frame deduplication purposes
    uint8_t toggle_bit:1;
    uint8_t is_end_of_transfer:1;
    uint8_t is_start_of_transfer:1;

    static constexpr TailByte from_bits(uint8_t bits) {
        return std::bit_cast<TailByte>(bits);
    }
    [[nodiscard]] static constexpr Self from_bits(const uint8_t bits){
        return std::bit_cast<Self>(bits);
    }

    [[nodiscard]] constexpr uint8_t to_bits() const {
        return std::bit_cast<uint8_t>(*this);
    }
};

static_assert(sizeof(TailByte) == 1);

// 4.2.2.3 Transfer payload decomposition
// The transport-layer MTU of Classic CAN-based implementations shall be 8 bytes (the maximum). The
// transport-layer MTU of CAN FD-based implementations should be 64 bytes (the maximum).
// CAN FD does not guarantee byte-level granularity of the CAN data field length. If the desired length of the CAN
// data field cannot be represented due to the granularity constraints, zero padding bytes are used.
// In single-frame transfers, padding bytes are inserted between the end of the payload and the tail byte.
// In multi-frame transfers, the transfer payload is appended with trailing zero padding bytes followed by the
// transfer CRC (section 4.2.2.4). All transport frames of a multi-frame transfer except the last one shall fully
// utilize the available data field capacity; hence, padding is unnecessary there. The number of padding bytes is
// computed so that the length granularity constraints for the last frame of the transfer are satisfied.

// 4.2.2.4
// Payload of multi-frame transfers is extended with a transfer CRC for validating the correctness of their re
// assembly. Transfer CRC is not used with single-frame transfers.
// The transfer CRC is computed over the entire payload of the multi-frame transfer plus the trailing padding
// bytes, if any. The resulting CRC value is appended to the transfer payload after the padding bytes (if any) in the
// big-endian byte order (most significant byte first)92.
// The transfer CRC function is CRC-16/CCITT-FALSE (section A.1).
}