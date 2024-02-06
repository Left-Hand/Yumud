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
#ifndef __CLOCK_H
#define __CLOCK_H

/* Includes ------------------------------------------------------------------*/

#include "ch32v20x.h"

#ifdef __cplusplus
extern "C" {
#endif
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

extern __IO uint64_t msTick;
extern uint32_t tick_per_ms;
extern uint32_t tick_per_us;

__attribute__ ((weak)) uint64_t GetTick(void);
uint64_t millis(void);
uint64_t micros(void);

void delay(uint32_t ms);

void Systick_Init(void);

static inline void delayMicroseconds(uint32_t) __attribute__((always_inline, unused));
static inline void delayMicroseconds(uint32_t us)
{
  __IO uint64_t currentTicks = SysTick->CNT;
  /* Number of ticks per millisecond */
  uint64_t tickPerMs = SysTick->CMP + 1;
  /* Number of ticks to count */
  uint64_t nbTicks = (us - 1) * tick_per_us;
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

static inline void delayNanoseconds(uint32_t ns) {
    for (volatile uint32_t i = 0; i < ns; i++) __asm__ volatile("");
}

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_H */


