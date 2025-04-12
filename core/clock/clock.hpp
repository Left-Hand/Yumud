#pragma once

#include "core/platform.hpp"

#include <functional>

namespace ymd{
    void bindSystickCb(std::function<void(void)> && cb);
    uint32_t millis(void);

    uint64_t micros(void);

    uint64_t nanos(void);

    void delay(uint32_t ms);
    void udelay(const uint32_t us);
    void delayNanoseconds(const uint32_t ns);
}



extern "C"{
void Systick_Init(void);
__interrupt void SysTick_Handler(void);
}