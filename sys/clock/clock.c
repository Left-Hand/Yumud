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

#include "clock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TICKS_PER_MS (F_CPU / 1000)
#define TICKS_PER_US (TICKS_PER_MS / 1000)

#define MICRO_TRIM 0
#define NANO_TRIM 300


volatile uint32_t msTick = 0;

void delay(const uint32_t ms)
{
  delayMicroseconds(ms * 1000);
}

void delayMicroseconds(const uint32_t us)
{
  uint32_t currentTicks = SysTick->CNT;
  /* Number of ticks per millisecond */
  uint32_t tickPerMs = SysTick->CMP + 1;
  /* Number of ticks to count */
  // uint64_t nbTicks = MAX(us - MICRO_TRIM, 0) * TICKS_PER_US;
  uint32_t nbTicks = us * TICKS_PER_US;
  /* Number of elapsed ticks */
  uint32_t elapsedTicks = 0;
  uint32_t oldTicks = currentTicks;
  do {
    currentTicks = SysTick->CNT;
    // elapsedTicks += (oldTicks < currentTicks) ? tickPerMs + oldTicks - currentTicks :
    //                 oldTicks - currentTicks;

    //increment
    elapsedTicks += (oldTicks <= currentTicks) ? currentTicks - oldTicks :
                    tickPerMs - oldTicks + currentTicks;

    oldTicks = currentTicks;
    } while (nbTicks > elapsedTicks);
}

volatile void delayNanoseconds(uint32_t ns) {
    __IO uint64_t currentTicks = SysTick->CNT;
    /* Number of ticks per millisecond */
    uint64_t tickPerMs = SysTick->CMP + 1;
    /* Number of ticks to count */
    uint64_t nbTicks = NANO_MUT(MAX(ns - NANO_TRIM, 0));
    /* Number of elapsed ticks */
    uint64_t elapsedTicks = 0;
    __IO uint64_t oldTicks = currentTicks;
    do {
        currentTicks = SysTick->CNT;
        // elapsedTicks += (oldTicks < currentTicks) ? tickPerMs + oldTicks - currentTicks :
        //                 oldTicks - currentTicks;

        //increment
        elapsedTicks += (oldTicks <= currentTicks) ? currentTicks - oldTicks :
                        tickPerMs - oldTicks + currentTicks;

        oldTicks = currentTicks;
    } while (nbTicks > elapsedTicks);
}

void Systick_Init(){
    static uint8_t initd = 0;
    if(initd) return;
    initd = 1;
    SysTick->SR  = 0;
    SysTick->CTLR= 0;
    SysTick->CNT = 0;
    SysTick->CMP = TICKS_PER_MS - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}


#ifdef __cplusplus
}
#endif