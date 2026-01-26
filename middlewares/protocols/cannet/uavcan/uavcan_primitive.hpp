#pragma once

#include <cstdint>
#include "transport/uavcan_transport_primitive.hpp"
#include "uavcan_f16.hpp"
#include "core/container/bits_set.hpp"


// https://dronecan.github.io/Specification/4.1_CAN_bus_transport_layer/
// https://cn.tmotor.com/uploadfile/2024/1101/20241101052544211.pdf

namespace ymd::uavcan{

using namespace ymd::literals;

struct [[nodiscard]] Signature{
    uint64_t bits;
};

//bits over 0b11111 is undefined behaviour
struct [[nodiscard]] TransferId final{
    using Self = TransferId;

    uint8_t bits;

    static constexpr Self from_bits(uint8_t bits){
        return TransferId{static_cast<uint8_t>(bits & 0b11111)};
    }

    constexpr Self rounded_inc() const {
        return Self{static_cast<uint8_t>((bits + 1) & 0b11111)};
    }

    constexpr Self & operator++() {
        auto & self = *this;
        self = self.rounded_inc();
        return self;
    }
};

}