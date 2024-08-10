#ifndef __USER_DEFS_H__

#define __USER_DEFS_H__

#ifdef CH32V20X
#define STRICT_IQ
#define UART_FIFO_BUF_SIZE 256
#define UART_DMA_BUF_SIZE 256

#define DEBUGGER uart1
#define LOGGER uart1
#define DEBUG_UART_BAUD 115200
// #include "v2defs.h"
#include "oled_defs.h"
#endif

#ifdef CH32V30X
#define UART_FIFO_BUF_SIZE 512
#define UART_DMA_BUF_SIZE 256

#define DEBUGGER uart2
#define LOGGER uart2
#define DEBUG_UART_BAUD 115200

#include "v3defs.h"
// #include "smc_defs.h"
#endif

#endif