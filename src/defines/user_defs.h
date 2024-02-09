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

#endif