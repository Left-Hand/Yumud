#ifndef __PLATFORM_H__

#define __PLATFORM_H__

#include "defines/defs.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef CH32V20X
#include <ch32v20x.h>
#include <ch32v20x_opa.h>
#endif

#ifdef CH32V30X
#include "sdk/ch32v307/Peripheral/inc/ch32v30x_conf.h"
#include "sdk/ch32v307/Peripheral/inc/ch32v30x.h"
#endif

#ifdef STM32F4XX

#endif


#if defined(USE_CH32V3_STD_LIB) || defined(USE_CH32V2_STD_LIB)
#define USE_CH32_STD_LIB
#endif


#endif