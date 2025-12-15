#pragma once

#include "primitive/can/can_id.hpp"

namespace ymd::cyphal::primitive{


struct [[nodiscard]] NodeId{
    using Self = NodeId;

    uint8_t bits;

    static constexpr NodeId from_u7(const uint8_t bits){
        return NodeId{static_cast<uint8_t>(bits & 0b1111111)};
    }

    static constexpr NodeId from_bits(const uint8_t bits){
        return from_u7(bits);
    }

    [[nodiscard]] constexpr uint8_t to_u7() const{
        return bits;
    }

    [[nodiscard]] constexpr uint8_t count() const{
        return to_u7();
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



}