#ifndef __USER_DEFS_H__

#define __USER_DEFS_H__

#ifdef CH32V20X
#include "v2defs.h"
#endif

#ifdef CH32V30X
#define UART_FIFO_BUF_SIZE 512
#define UART_DMA_BUF_SIZE 256
// #include "v3defs.h"
#include "smc_defs.h"
#endif


#endif