#ifndef __USER_DEFS_H__

#define __USER_DEFS_H__

#include <ch32v20x.h>

#define USE_IQ
// #define USE_DOUBLE
#define USE_LOG
#define USE_STDMATH
#define EXTRA_IQ
// #define STRICT_IQ

//defines for iqlib

// #define SPI1_USE_DMA

#define UART1_Baudrate (115200 * 4)
#define UART2_Baudrate (115200)

#define I2C_SW_PORT GPIOB
#define I2C_SW_SCL GPIO_Pin_6
#define I2C_SW_SDA GPIO_Pin_7

#define I2S_SW_PORT GPIOB
#define I2S_SW_SCK GPIO_Pin_10
#define I2S_SW_SDA GPIO_Pin_11
#define I2S_SW_WS GPIO_Pin_1

#define SPI1_BaudRate (144000000/2)
#define SPI2_BaudRate (144000000/2)

#define I2C_BaudRate 400000


#define RES_PORT GPIOA
#define RES_PIN GPIO_Pin_12

#define SPI1_REMAP_ENABLE ENABLE

#if SPI1_REMAP_ENABLE == ENABLE
    #define SPI1_CS_Port GPIOA
    #define SPI1_CS_Pin GPIO_Pin_15

    #define SPI1_Port GPIOB
    #define SPI1_SCLK_Pin GPIO_Pin_3
    #define SPI1_MISO_Pin GPIO_Pin_4
    #define SPI1_MOSI_Pin GPIO_Pin_5
    #define SPI1_REMAP GPIO_Remap_SPI1
#else
    #define SPI1_CS_Port GPIOA
    #define SPI1_CS_Pin GPIO_Pin_4

    #define SPI1_Port GPIOA
    #define SPI1_SCLK_Pin GPIO_Pin_5
    #define SPI1_MISO_Pin GPIO_Pin_6
    #define SPI1_MOSI_Pin GPIO_Pin_7
    #define SPI1_REMAP GPIO_Remap_SPI1
#endif

#define SPI2_CS_Port GPIOB
#define SPI2_CS_Pin GPIO_Pin_12

#define SPI2_Port GPIOB
#define SPI2_SCK_Pin GPIO_Pin_13
#define SPI2_MISO_Pin GPIO_Pin_14
#define SPI2_MOSI_Pin GPIO_Pin_15

#define I2C1_REMAP_ENABLE DISABLE

#if I2C1_REMAP_ENABLE == ENABLE
    #define I2C1_Port GPIOB
    #define I2C1_SCL_Pin GPIO_Pin_8
    #define I2C1_SDA_Pin GPIO_Pin_9
    #define I2C1_REMAP GPIO_Remap_I2C1
#else
    #define I2C1_Port GPIOB
    #define I2C1_SCL_Pin GPIO_Pin_6
    #define I2C1_SDA_Pin GPIO_Pin_7
    #define I2C1_REMAP GPIO_Remap_I2C1
#endif

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