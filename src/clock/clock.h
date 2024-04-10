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

#include "src/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define TIMESTAMP_BEGIN(x) (uint32_t __time_begin_t__ = x;)
// #define TIMESTAMP_SINCE(x) (uint32_t __time_current_t__ = x; __time_current_t__ + (__time_current_t__ ^ __time_begin_t__) & 0x8000 ?  __time_begin_t__ : -__time_begin_t__)
#define TIMESTAMP_BEGIN(x) uint32_t time_begin = (x)
#define TIMESTAMP_SINCE(x) (uint32_t time_current = (x); \
            (time_current + (time_current ^ time_begin) & 0x8000? time_begin : -time_begin))

extern int tick_per_ms;
extern int tick_per_us;
#define NanoMut(x) ( x * 1000 / 144)

#define MicroTrim 0
#define NanoTrim 300

extern volatile uint32_t msTick;

__attribute__ ((weak)) uint32_t GetTick(void);
__attribute__ ((weak)) void SetTick(uint32_t _tick);
uint32_t millis(void);
uint64_t micros(void);
uint64_t nanos(void);

void delay(uint32_t ms);

void Systick_Init(void);

void delayMicroseconds(uint32_t us);
void delayNanoseconds(uint32_t ns);

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_H */


