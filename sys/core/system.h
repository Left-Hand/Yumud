#pragma once

#include "platform.h"
#include "kernel/clock.h"

#ifdef N32G45X
#define M_RCC_CONFIGER RCC_ConfigHclk
#else
#define M_RCC_CONFIGER RCC_HCLKConfig
#endif

#ifdef N32G45X
#define M_PCLK1_CONFIGER RCC_ConfigPclk1
#else
#define M_PLCK1_CONFIGER RCC_PCLK1Config
#endif

#ifdef N32G45X
#define M_PCLK2_CONFIGER RCC_ConfigPclk2
#else
#define M_PLCK2_CONFIGER RCC_PCLK2Config
#endif

#ifdef N32G45X
#define M_CLOCK_TYPEDEF RCC_ClocksType
#else
#define M_CLOCK_TYPEDEF RCC_ClocksTypeDef
#endif

#ifdef N32G45X
#define M_RCC_CLK_GETTER RCC_GetClocksFreqValue
#else
#define M_RCC_CLK_GETTER RCC_GetClocksFreq
#endif
void prework(void);