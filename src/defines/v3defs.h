#pragma once

#include "sys/platform.h"

#define F_CPU 144000000

#define USE_IQ

#define IQ_USE_LOG

#define EXTRA_IQ

#define HAVE_UART1
#define HAVE_UART2
#define HAVE_UART3
#define HAVE_UART4
#define HAVE_UART5
#define HAVE_UART6
#define HAVE_UART7
#define HAVE_UART8

#define UART1_REMAP 1
#define UART2_REMAP 0
#define UART3_REMAP 0
#define UART4_REMAP 0
#define UART5_REMAP 0
#define UART6_REMAP 0
#define UART7_REMAP 0
#define UART8_REMAP 0

#define HAVE_SPI1
#define SPI1_REMAP 1

// #define HAVE_SPI2

#define HAVE_TIM1
#define TIM1_REMAP 0

#define HAVE_TIM2
#define TIM2_REMAP 1

#define HAVE_TIM3
#define TIM3_REMAP 0

#define HAVE_TIM4
#define TIM4_REMAP 0

#define HAVE_TIM5
#define TIM5_REMAP 0

#define HAVE_TIM6
#define TIM6_REMAP 0

#define HAVE_TIM7
#define TIM7_REMAP 0

#define HAVE_TIM8
#define TIM8_REMAP 0

#define HAVE_CAN1
#define CAN1_REMAP 0

// #define HAVE_I2C1
#define I2C1_REMAP 0
// #define HAVE_I2C2
#define I2C2_REMAP 0

#define HAVE_OPA1
#define HAVE_OPA2


#define DEBUGGER uart1
// #define DEBUGGER sdi
#define LOGGER uart1