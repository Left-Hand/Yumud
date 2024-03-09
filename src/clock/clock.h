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
void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

extern __IO uint32_t msTick;
extern int tick_per_ms;
extern int tick_per_us;
#define NanoMut(x) ( x * 1000 / 144)

#define MicroTrim 0
#define NanoTrim 300

__attribute__ ((weak)) uint32_t GetTick(void);
uint32_t millis(void);
uint64_t micros(void);
uint64_t nanos(void);

void delay(uint32_t ms);

void Systick_Init(void);

void delayMicroseconds(uint32_t us);
void delayNanoseconds(uint32_t ns);

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_H */


