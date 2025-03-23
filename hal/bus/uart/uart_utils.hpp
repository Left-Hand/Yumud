#pragma once

#include <cstdint>

namespace ymd::hal{
    class Gpio;

    enum class UartParity{
        None    = 0x0000,
        Even    = 0x0400,
        Odd     = 0x0600
    };
    
}