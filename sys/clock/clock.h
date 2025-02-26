#pragma once

#include "sys/core/platform.h"


#ifdef __cplusplus
#include <functional>
namespace ymd{
    void bindSystickCb(std::function<void(void)> && cb);
}
extern "C"{
#endif

uint32_t millis(void);

uint64_t micros(void);

uint64_t nanos(void);

void delay(uint32_t ms);
void delayMicroseconds(const uint32_t us);
void delayNanoseconds(const uint32_t ns);

void Systick_Init(void);
__interrupt void SysTick_Handler(void);

#ifdef __cplusplus
}

#endif