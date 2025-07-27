#pragma once

namespace ymd{
struct NodeId{
    static constexpr NodeId from_u8(const uint8_t raw) {
        return NodeId{raw};
    }

    constexpr uint8_t as_u8() const {
        return id;
    }

    uint8_t id;
};
}