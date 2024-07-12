#ifndef __SYSTEM_HPP__

#define __SYSTEM_HPP__

#include "core/platform.h"
#include "../Yumud/types/real.hpp"
#include "kernel/clock.h"
#include "enums.hpp"

__fast_inline uint64_t operator"" _KHz(uint64_t x){
    return x * 1000;
}

__fast_inline uint64_t operator"" _MHz(uint64_t x){
    return x * 1000000;
}

__fast_inline uint64_t operator"" _GHz(uint64_t x){
    return x * 1000000000;
}

__fast_inline uint64_t operator"" _KB(uint64_t x){
    return x << 10;
}

__fast_inline uint64_t operator"" _MB(uint64_t x){
    return x << 20;
}

__fast_inline uint64_t operator"" _GB(uint64_t x){
    return x << 30;
}

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

namespace Sys{
    extern real_t t;

    namespace Misc{
        void prework();
        void reset();
    };
    namespace Chip{
        uint64_t getChipId();
        uint32_t getChipIdCrc();
        uint32_t getFlashSize();
    };

    namespace Clock{
        void delayMs(const uint32_t ms);
        void delayUs(const uint32_t us);

        uint32_t getSystemFreq();
        uint32_t getAPB1Freq();
        void setAPB1Freq(const uint32_t ferq);
        uint32_t getAPB2Freq();
        void setAPB2Freq(const uint32_t ferq);
        uint32_t getAHBFreq();
        void setAHBFreq(const uint32_t ferq);

        void setAPB1Div(const uint8_t div);
        void setAPB2Div(const uint8_t div);
        void setAHBDiv(const uint8_t div);

        void reCalculateTime();
        real_t getCurrentSeconds();
    };
};


extern "C"{
    __interrupt void NMI_Handler(void);
    __interrupt void HardFault_Handler(void);
}

#endif // !__SYSTEM_HPP__