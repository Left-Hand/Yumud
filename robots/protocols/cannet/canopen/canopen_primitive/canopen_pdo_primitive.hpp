#pragma once

#include "canopen_primitive_base.hpp"

namespace ymd::canopen::primitive{

struct [[nodiscard]] PdoMapping{
    uint8_t bits;
    uint8_t subindex;
    uint8_t index;
    static constexpr PdoMapping from_u32(const uint32_t map){
        PdoMapping self;
        self.bits = (map & 0xFF);
        self.subindex = ((map >> 8) & 0xFF);
        self.index = ((map >> 16) & 0xFF);
        return self;
    }

    constexpr uint32_t to_u32(const PdoMapping & map){ 
        return (map.bits | (map.subindex << 8) | (map.index << 16));
    }

};

struct [[nodiscard]] PdoParaments{
    OdPreIndex idx;
    CobId cob_id;
    uint8_t transmission_type;
    uint8_t inhibit_time;
    uint16_t event_timer;
    PdoMapping mapping;
};

}