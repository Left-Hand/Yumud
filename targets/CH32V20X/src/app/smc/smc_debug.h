#pragma once

#include "../sys/debug/debug_inc.h"
#include "../hal/bkp/bkp.hpp"
#include "../thirdparty/enums/enum.h"

namespace SMC{


BETTER_ENUM(RunStatus, uint8_t,
    NONE = 0,
    BEG,
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

__inline auto getRunStatus() -> RunStatus{
    uint16_t temp = (uint16_t)(bkp[1]);
    return RunStatus::_from_integral_unchecked((int)temp);
}

__inline void recordRunStatus(const RunStatus status){
    static auto & runStatusReg = bkp[1];
    
    runStatusReg = (uint8_t(status));
    // DEBUG_PRINTLN(status._to_string(), getRunStatus()._to_string());
    // DEBUG_PRINTLN((uint8_t)(status), (uint16_t)runStatusReg);
}


}