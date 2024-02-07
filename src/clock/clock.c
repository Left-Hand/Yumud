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
  if (ms != 0) {
    uint64_t start = millis();
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

void Systick_Init(void){
    static uint8_t inited = 0;
    if(inited) return;
    inited = 1;

    tick_per_ms = SystemCoreClock / 1000;
    tick_per_us = tick_per_ms / 1000;
    SysTick->SR  = 0;
    SysTick->CTLR= 0;
    SysTick->CNT = 0;
    SysTick->CMP = tick_per_ms - 1;
    SysTick->CTLR= 0xF;

    NVIC_SetPriority(SysTicK_IRQn,0xFF);
    NVIC_EnableIRQ(SysTicK_IRQn);
}

#ifdef __cplusplus
}
#endif