#pragma once

#include "../sys/core/platform.h"

#define F_CPU 144000000

#define USE_IQ

#define IQ_USE_LOG

#define EXTRA_IQ

#define HAVE_UART1
#define HAVE_UART2


#define UART1_REMAP 1
#define UART2_REMAP 1

#define HAVE_SPI1
#define SPI1_REMAP 1


#define HAVE_TIM1
#define TIM1_REMAP 0

#define HAVE_TIM2
#define TIM2_REMAP 1


#define HAVE_TIM3
#define TIM3_REMAP 0

#define HAVE_TIM4
#define TIM4_REMAP 0


#define HAVE_CAN1
#define CAN1_REMAP 1

// #define HAVE_I2C1
#define I2C1_REMAP 0
// #define HAVE_I2C2
#define I2C2_REMAP 0

#define HAVE_OPA1
#define HAVE_OPA2
