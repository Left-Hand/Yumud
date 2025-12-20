#pragma once

#include "canopen_primitive_base.hpp"

namespace ymd::canopen::primitive{

struct [[nodiscard]] PdoMapping{
    uint8_t bits;
    uint8_t sub_idx;
    uint16_t pre_idx;
    static constexpr PdoMapping from_u32(const uint32_t map){
        PdoMapping self;
        self.bits = (map & 0xFF);
        self.sub_idx = ((map >> 8) & 0xFF);
        self.pre_idx = ((map >> 16) & 0xFF);
        return self;
    }

    constexpr uint32_t to_u32(const PdoMapping & map){ 
        return (map.bits | (map.sub_idx << 8) | (map.pre_idx << 16));
    }

};

struct [[nodiscard]] PdoParaments{
    OdPreIndex pre_idx;
    CobId cob_id;
    uint8_t transmission_type;
    uint8_t inhibit_time;
    uint16_t event_timer;
    PdoMapping mapping;
};

}