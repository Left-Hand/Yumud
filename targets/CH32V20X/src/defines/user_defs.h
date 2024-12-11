#pragma once

#ifdef CH32V20X
#define STRICT_IQ
#define UART_FIFO_BUF_SIZE 256
#define UART_DMA_BUF_SIZE 256

#define DEBUGGER_INST uart1
#define LOGGER_INST uart1
#define DEBUG_UART_BAUD 576000
#include "embd_defs.h"
#endif

#ifdef CH32V30X
#define UART_FIFO_BUF_SIZE 512
#define UART_DMA_BUF_SIZE 256

#define DEBUGGER_INST uart7
#define LOGGER_INST uart7
#define DEBUG_UART_BAUD 115200

#include "v3defs.h"
// #include "smc_defs.h"
#endif