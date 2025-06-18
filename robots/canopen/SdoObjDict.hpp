#pragma once

#include "ObjectDict.hpp"


namespace ymd::canopen{

struct SdoObjDict:public StaticObjectDictBase{
    CobId c2s_cobid = CobId::from_u16(0);
    CobId s2c_cobid = CobId::from_u16(0);
    uint16_t heartbeat_time = 0;
    uint16_t node_guarding_time = 0;
    uint32_t sync_period = 0;
    uint32_t sync_window_length = 0;
    uint32_t emergency_consumer_cobid = 0;
    uint32_t emergency_producer_cobid = 0;

    Option<SubEntry> get_sub_entry(const Didx didx) ;
};


}