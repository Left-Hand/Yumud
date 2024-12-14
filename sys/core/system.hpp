#pragma once

#include "sys/core/platform.h"
#include "sys/clock/clock.h"
#include "sys/clock/clock.hpp"
#include "sys/math/real.hpp"

namespace Sys{
    namespace Misc{
        void prework();
        void reset();
    };

    namespace Chip{
        uint64_t getChipId();
        uint32_t getChipIdCrc();
        uint32_t getFlashSize();
        uint64_t getMacAddress();
    };

    namespace Exception{
        __inline void disableInterrupt(){__disable_irq();}
        __inline void enableInterrupt(){__enable_irq();}
        bool isInterruptPending();
        bool isIntrruptActing();
        uint8_t getInterruptDepth();
    }

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
        void reCalculateTimeMs();
    };
};


