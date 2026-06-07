#pragma once

#include "canopen_primitive_base.hpp"

namespace ymd::canopen::primitive{

struct [[nodiscard]] PdoMapping final{
    // uint8_t bits;
    // uint8_t minor_index;
    // uint16_t major_index;


    uint32_t bits;


    // constexpr FunctionCode func_code() const noexcept{
    //     return make_bitfield_proxy<7, 11, FunctionCode>(&bits).get();
    // }

    // constexpr NodeId node_id() const noexcept{
    //     return make_bitfield_proxy<0, 7, NodeId>(&bits).get();
    // }


    // constexpr uint32_t to_u32(const PdoMapping & map){ 
    //     return (map.bits | (map.minor_index << 8) | (map.major_index << 16));
    // }

};

struct [[nodiscard]] PdoParaments final{
    OdMajorIndex major_index;
    CobId cob_id;
    uint8_t transmission_type;
    uint8_t inhibit_time;
    uint16_t event_timer;
    PdoMapping mapping;
};

}