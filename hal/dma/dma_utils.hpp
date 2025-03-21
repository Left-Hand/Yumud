#pragma once

#include <cstdint>
#include "core/sdk.hpp"

namespace ymd::hal{
    enum class DmaMode:uint8_t{
        toMem = DMA_DIR_PeripheralSRC,
        toPeriph = DMA_DIR_PeripheralDST,
        synergy,
        distribute,

        toMemCircular = DMA_DIR_PeripheralSRC | 0x80,
        toPeriphCircular = DMA_DIR_PeripheralDST | 0x80,
        synergyCircular,
        distributeCircular,
        automatic
    };

    enum class DmaPriority:uint16_t{
        low = DMA_Priority_Low,
        medium = DMA_Priority_Medium,
        high = DMA_Priority_High,
        ultra = DMA_Priority_VeryHigh
    };
}