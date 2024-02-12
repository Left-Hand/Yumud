#ifndef __USER_DEFS_H__

#define __USER_DEFS_H__

#define USE_IQ
// #define USE_DOUBLE
#define USE_LOG
#define USE_STDMATH
#define EXTRA_IQ
// #define STRICT_IQ

//defines for iqlib

#define SPI1_Prescaler SPI_BaudRatePrescaler_2
// #define SPI1_USE_DMA

#define UART1_Baudrate (115200 * 4)
#define UART2_Baudrate (115200)

#define W (240)
#define H (240)

//#defines for LCD

#define SPI2_BaudRate SPI_BaudRatePrescaler_256
#define I2C_BaudRate 400000

#define LCD_DC_PORT GPIOA
#define LCD_DC_PIN GPIO_Pin_15

#define LCD_RES_PORT GPIOA
#define LCD_RES_PIN GPIO_Pin_12

#define LCD_ON_DATA LCD_DC_PORT -> BSHR = LCD_DC_PIN;
#define LCD_ON_COMMAND LCD_DC_PORT -> BCR = LCD_DC_PIN;

#define LCD_SET_RES LCD_DC_PORT -> BSHR = LCD_RES_PIN;
#define LCD_RESET_RES LCD_DC_PORT -> BCR = LCD_RES_PIN;

#define SPI1_CS_Port GPIOA
#define SPI1_CS_Pin GPIO_Pin_15
#define SPI1_SCLK_Pin GPIO_Pin_3
#define SPI1_MOSI_Pin GPIO_Pin_5
#define SPI1_MISO_Pin GPIO_Pin_4
#define SPI1_Remap GPIO_Remap_SPI1

#endif