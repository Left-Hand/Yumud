#ifndef __USER_DEFS_H__

#define __USER_DEFS_H__

#include <ch32v20x.h>
// #include <core_riscv.h>

#define USE_IQ
// #define USE_DOUBLE
#define USE_LOG
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
#define HAVE_CAN1


#define I2C_SW_PORT GPIOB
#define I2C_SW_SCL GPIO_Pin_6
#define I2C_SW_SDA GPIO_Pin_7

#define I2S_SW_PORT GPIOB
#define I2S_SW_SCK GPIO_Pin_10
#define I2S_SW_SDA GPIO_Pin_11
#define I2S_SW_WS GPIO_Pin_1

#define SPI1_BaudRate (144000000/256)
#define SPI2_BaudRate (144000000/8)

#define I2C_BaudRate 400000


#define RES_PORT GPIOA
#define RES_PIN GPIO_Pin_12

#define BUILTIN_LED_PORT GPIOC
#define BUILTIN_RedLED_PIN GPIO_Pin_13
#define BUILTIN_GreenLED_PIN GPIO_Pin_14
#define BUILTIN_BlueLED_PIN GPIO_Pin_15


#define ST7789_DC_Port SPI2_Port
#define ST7789_DC_Pin SPI2_MISO_Pin

#define ST7789_ON_DATA \
ST7789_DC_Port -> BSHR = ST7789_DC_Pin;

#define ST7789_ON_CMD \
ST7789_DC_Port -> BCR = ST7789_DC_Pin;

#endif