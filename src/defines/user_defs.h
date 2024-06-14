#ifndef __USER_DEFS_H__

#define __USER_DEFS_H__

#include <ch32v20x.h>
#include <ch32v20x_opa.h>
#include <ch32v20x_iwdg.h>
// #include <core_riscv.h>

#define USE_IQ
// #define USE_DOUBLE
#define IQ_USE_LOG
#define USE_STDMATH
#define EXTRA_IQ
// #define STRICT_IQ

//defines for iqlib

// #define SPI1_USE_DMA

#define HAVE_I2C1
#define HAVE_UART1
#define HAVE_UART2

#define HAVE_SPI1
#define HAVE_SPI2

#define HAVE_TIM1
#define HAVE_TIM2
#define TIM2_REMAP (GPIO_PartialRemap2_TIM2)
#define TIM2_REMAP_ENABLE ENABLE

#define HAVE_TIM3
#define HAVE_TIM4
#define TIM3_REMAP_ENABLE DISABLE


#define HAVE_CAN1
#define CAN1_REMAP_ENABLE DISABLE

#define HAVE_I2C1
#define I2C1_REMAP_ENABLE (DISABLE)

#define HAVE_OPA1
#define HAVE_OPA2

#endif