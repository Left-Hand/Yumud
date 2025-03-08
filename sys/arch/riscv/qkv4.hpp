#pragma once

#include <cstdint>

#define QKV4_REG_DEF(name) static inline volatile name##_Def * name = reinterpret_cast<name##_Def *>(name##_Def::address);
namespace QingKeV4{
    struct PFIC_CFGR_Def{
        static constexpr uint32_t address = 0xE000E04C;
        uint32_t NESTSTA:8;
        uint32_t GACTSTA:1;
        uint32_t GPENDSTA:1;
        uint32_t:22;
    };

    QKV4_REG_DEF(PFIC_CFGR)

    __inline bool isInterruptPending(){
        return PFIC_CFGR->GPENDSTA;
    }

    __inline bool isIntrruptActing(){
        return PFIC_CFGR->GACTSTA;
    }

    __inline uint8_t getInterruptDepth(){
        return PFIC_CFGR->NESTSTA;
    }
}