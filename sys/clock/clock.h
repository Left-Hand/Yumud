/**
 *******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * This software component is licensed by WCH under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 *******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once
/* Includes ------------------------------------------------------------------*/

#include "../sys/core/platform.h"

#ifdef __cplusplus
#include "sys/math/iq/iqt.hpp"
#include <functional>
namespace Sys{
extern iq_t t;
};
void bindSystickCb(std::function<void(void)> && cb);
extern "C" {
#endif

#ifdef CH32V20X

#define TIMESTAMP_BEGIN(x) uint32_t time_begin = (x)
#define TIMESTAMP_SINCE(x) (uint32_t time_current = (x); \
            (time_current + (time_current ^ time_begin) & 0x8000? time_begin : -time_begin))
#else
#define TIMESTAMP_BEGIN(x) (uint32_t __time_begin_t__ = x;)
#define TIMESTAMP_SINCE(x) (uint32_t __time_current_t__ = x; \
            __time_current_t__ + (__time_current_t__ ^ __time_begin_t__) & 0x8000 ?  __time_begin_t__ : -__time_begin_t__)
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

#define NANO_MUT(x) ( x * 1000 / (F_CPU / 1000000))

extern volatile uint32_t msTick;

__fast_inline uint32_t millis(void){return msTick;}

__fast_inline static uint64_t micros(void){
    M_SYSTICK_DISER;
    __IO uint64_t m = msTick;
    __IO uint64_t ticks = M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (m * 1000 + ticks / TICKS_PER_US);
}

__fast_inline static uint64_t nanos(){
    M_SYSTICK_DISER;
    __IO uint64_t m = msTick;
    __IO uint64_t ticks = M_SYSTICK_CNT;
    M_SYSTICK_ENER;

    return (m * 1000000 + NANO_MUT(ticks));
}

void delay(uint32_t ms);


void delayMicroseconds(const uint32_t us);
void delayNanoseconds(const uint32_t ns);

void Systick_Init(void);
__interrupt void SysTick_Handler(void);


#undef TIMESTAMP_BEGIN
#undef TIMESTAMP_SINCE

#undef TICKS_PER_MS
#undef TICKS_PER_US

#undef M_SYSTICK_CNT
#undef M_SYSTICK_DISER
#undef M_SYSTICK_ENER


#ifdef __cplusplus
}
#endif

