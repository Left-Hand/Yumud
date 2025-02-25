#pragma once

#include "sys/core/platform.h"

namespace sys{
    void preinit();
    void reset();

    void exit();
    void halt();
    void jumpto(const uint32_t addr);

    namespace Chip{
        uint64_t getChipId();
        uint32_t getChipIdCrc();
        uint32_t getFlashSize();
        uint64_t getMacAddress();
    };

    namespace Exception{
        void disableInterrupt();
        void enableInterrupt();
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

        // void reCalculateTime();
        // void reCalculateTimeMs();
    };
};


