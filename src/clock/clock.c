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
#include "core_riscv.h"
#include "ch32v20x_rcc.h"
// #include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

int tick_per_ms = 0;
int tick_per_us = 0;


__IO uint32_t msTick=0;
__attribute__ ((weak)) uint32_t GetTick(void)
{
  return msTick;
}


/**
  * @brief  Function called wto read the current millisecond
  * @param  None
  * @retval None
  */
uint32_t millis(void)
{
  return msTick;
}

uint64_t micros(void)
{
    __disable_irq();
    uint64_t m = GetTick();
    __IO uint64_t ticks = SysTick->CNT;
    __enable_irq();

    return (m * 1000 + ticks / tick_per_us);

}

uint64_t nanos(){
    __disable_irq();
    uint64_t m = GetTick();
    __IO uint64_t ticks = SysTick->CNT;
    __enable_irq();

    return (m * 1000000 + NanoMut(ticks));
}

void delay(uint32_t ms)
{
  delayMicroseconds(ms * 1000);
}

void SysTick_Handler(void)
{
  msTick+=1;
  SysTick->SR = 0;
}

void delayMicroseconds(uint32_t us)
{
  __IO uint64_t currentTicks = SysTick->CNT;
  /* Number of ticks per millisecond */
  uint64_t tickPerMs = SysTick->CMP + 1;
  /* Number of ticks to count */
  uint64_t nbTicks = MAX(us - MicroTrim, 0) * tick_per_us;
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

volatile void delayNanoseconds(uint32_t ns) {
    __IO uint64_t currentTicks = SysTick->CNT;
    /* Number of ticks per millisecond */
    uint64_t tickPerMs = SysTick->CMP + 1;
    /* Number of ticks to count */
    uint64_t nbTicks = NanoMut(MAX(ns - NanoTrim, 0));
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

#ifdef __cplusplus
}
#endif