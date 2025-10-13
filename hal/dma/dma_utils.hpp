#pragma once

#include <cstdint>


namespace ymd::hal{

enum class DmaMode:uint8_t{
    toMem = 0,
    toPeriph,
    synergy,
    distribute,

    toMemCircular,
    toPeriphCircular,
    synergyCircular,
    distributeCircular,
    automatic
};

enum class DmaPriority:uint16_t{
    Low = 0,
    Medium = 1,
    High = 2,
    Ultra = 3
};

}