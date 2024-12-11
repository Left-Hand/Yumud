#pragma once

#include "sys/core/platform.h"

#ifdef __cplusplus
extern "C"{
#endif

#define TICKS_PER_MS (F_CPU / 1000)
#define TICKS_PER_US (TICKS_PER_MS / 1000)

#ifdef N32G45X
#define M_SYSTICK_CNT SysTick->VAL
#else
#define M_SYSTICK_CNT SysTick->CNT
#define M_SYSTICK_DISER    NVIC_DisableIRQ(SysTicK_IRQn);
#define M_SYSTICK_ENER    NVIC_EnableIRQ(SysTicK_IRQn);
#endif

#define NANO_MUT(x) (((x) * 1000) / TICKS_PER_US)

extern volatile uint32_t msTick;

__fast_inline static uint32_t millis(void){return msTick;}

__fast_inline static uint64_t micros(void){
    M_SYSTICK_DISER;
    uint32_t m = msTick;
    uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (m * 1000 + ticks / TICKS_PER_US);
}

__fast_inline static uint64_t nanos(){
    M_SYSTICK_DISER;
    uint32_t m = msTick;
    uint32_t ticks = (uint32_t)M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (m * 1000000 + NANO_MUT(ticks));
}

void delay(uint32_t ms);


void delayMicroseconds(const uint32_t us);
void delayNanoseconds(const uint32_t ns);

void Systick_Init(void);
__interrupt void SysTick_Handler(void);

#undef TICKS_PER_MS
#undef TICKS_PER_US

#undef M_SYSTICK_CNT
#undef M_SYSTICK_DISER
#undef M_SYSTICK_ENER

#ifdef __cplusplus
}
#endif