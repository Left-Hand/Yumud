#pragma once

#include <cstdint>
namespace yumud::DmaUtils{
    enum class Mode:uint8_t{
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

    enum class Priority:uint16_t{
        low = DMA_Priority_Low,
        medium = DMA_Priority_Medium,
        high = DMA_Priority_High,
        ultra = DMA_Priority_VeryHigh
    };
}