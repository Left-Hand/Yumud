#pragma once

#include "../sys/debug/debug_inc.h"
#include "../hal/bkp/bkp.hpp"
#include "../thirdparty/enums/enum.h"

namespace SMC{


BETTER_ENUM(RunStatus, uint8_t,
    BEG = 0,
    CLI,
    INPUT,
    EVENTS,

    IMG_B,
    IMG_E,

    SEED_B,
    SEED_E,

    COAST_B,
    COAST_E,

    DP_B,
    DP_E,

    VEC_B,
    VEC_E,

    CORNER_B,
    CORNER_L,
    CORNER_R,
    CORNER_E,


    ELEMENT_B,
    ELEMENT_E,

    BARRIER_B,
    BARRIER_E,

    CROSS_B,
    CROSS_E,

    RING_B,
    RING_E,



    SEGMENT,
    SEGMENT_E,
    FANS_B,

    FANS_E,
    END
)


extern BkpItem & runStatusReg;

__inline void recordRunStatus(const RunStatus status){
    runStatusReg = uint16_t(uint8_t(status));
}

}