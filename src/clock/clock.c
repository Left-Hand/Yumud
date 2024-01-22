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

#define TICK_FREQ_1KHz    1L
// #define TICK_FREQ_100Hz   10L
// #define TICK_FREQ_10Hz    100L 


__IO uint64_t msTick=0;
__attribute__ ((weak)) uint64_t GetTick(void)
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
  return GetTick();
}

uint32_t micros(void)
{
  
  uint64_t m0 = GetTick();
  __IO uint64_t u0 = SysTick->CNT;
  uint64_t m1 = GetTick();
  __IO uint32_t u1 = SysTick->CNT;   //may be a interruption
   uint64_t tms = SysTick->CMP + 1;

  if (m1 != m0) {
    return (m1 * 1000 + ((tms - u1) * 1000) / tms);
  } else {
    return (m0 * 1000 + ((tms - u0) * 1000) / tms);
  }
}

void delay(uint32_t ms)
{
  if (ms != 0) {
    uint32_t start = millis();
    do {
      // yield();
    } while (millis() - start < ms);
  }
}

void SysTick_Handler(void)
{
  msTick+=1;
  SysTick->SR = 0;
}


#ifdef __cplusplus
}
#endif