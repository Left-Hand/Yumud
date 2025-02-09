#pragma once

#include "ObjectDict.hpp"


namespace ymd::canopen{

struct PdoMappingItem{
    uint8_t bits;
    uint8_t subindex;
    uint8_t index;

    PdoMappingItem(const int map):
        bits(map & 0xFF),
        subindex((map >> 8) & 0xFF),
        index((map >> 16) & 0xFF){
    }
};


struct PdoObjDict:public StaticObjectDictBase{
    uint16_t tx_param;
    uint32_t tx_mapping;
    uint16_t rx_param;
    uint32_t rx_mapping;

    PdoObjDict() = default;

    std::optional<SubEntry> find(const Didx didx) final override;
};
        
    

}