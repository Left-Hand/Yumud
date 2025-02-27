#pragma once

#include "ObjectDict.hpp"


namespace ymd::canopen{

struct SdoObjDict:public StaticObjectDictBase{
    CobId c2s_cobid = 0;
    CobId s2c_cobid = 0;
    uint16_t heartbeat_time = 0;
    uint16_t node_guarding_time = 0;
    uint32_t sync_period = 0;
    uint32_t sync_window_length = 0;
    uint32_t emergency_consumer_cobid = 0;
    uint32_t emergency_producer_cobid = 0;

    std::optional<SubEntry> find(const Didx didx) final override;
};


}