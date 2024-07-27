#pragma once

#include "../sys/core/platform.h"
#include "../../types/real.hpp"

namespace Sys{
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
    };
};


extern "C"{
    __interrupt void NMI_Handler(void);
    __interrupt void HardFault_Handler(void);
}
