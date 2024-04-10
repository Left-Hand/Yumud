#ifndef __SYSTEM_HPP__

#define __SYSTEM_HPP__

#include "platform.h"
#include "types/real.hpp"
#include "clock/clock.h"

inline unsigned long long operator"" _kHz(unsigned long long x)
{
    return x * 1000;
}

namespace Sys{
    extern real_t t;

    void Reset();

    void reCalculateTime();
    real_t getCurrentSeconds();
    uint64_t getChipId();
    uint32_t getChipIdCrc();
};
#endif // !__SYSTEM_HPP__