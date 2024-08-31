#ifndef __PLATFORM_H__

#define __PLATFORM_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "src/defines/defs.h"

#if defined(CH32V203)
#ifndef CH32V20X
#define CH32V20X
#endif
#endif 

#if defined(CH32V307)
#ifndef CH32V30X
#define CH32V30X
#endif
#endif 

#ifdef CH32V20X
#ifndef USE_CH32V2_STD_LIB
#define  USE_CH32V2_STD_LIB
#endif
#define ARCH_QKV4
#endif

#ifdef CH32V30X
#ifndef USE_CH32V3_STD_LIB
#define  USE_CH32V3_STD_LIB
#endif
#define ARCH_QKV4
#endif

#ifdef STM32F4XX

#endif


#ifdef N32G45X
#include "sdk/n32g45x/CMSIS/core/core_cm4.h"
#include "sdk/n32g45x/CMSIS/device/n32g45x.h"
#endif


#if defined(USE_CH32V3_STD_LIB) || defined(USE_CH32V2_STD_LIB)
#define USE_CH32_STD_LIB
#endif

#ifdef USE_CH32V3_STD_LIB
#include "sdk/ch32v307/Peripheral/inc/ch32v30x_conf.h"
#include "sdk/ch32v307/Peripheral/inc/ch32v30x.h"
#include "sdk/ch32v307/system_ch32v30x.h"
#include "sdk/ch32v307/Core/core_riscv.h"
#endif

#ifdef USE_CH32V2_STD_LIB
#include "sdk/ch32v203/Peripheral/inc/ch32v20x_conf.h"
#include "sdk/ch32v203/system_ch32v20x.h"
#include "sdk/ch32v203/Core/core_riscv.h"
#endif


#if defined(__cplusplus)

#include "../sys/constants/enums.hpp"
#include "../sys/constants/concepts.hpp"
#include "../sys/constants/units.hpp"

#include <type_traits>
#include <limits>

#ifdef ARCH_QKV4
#include "arch/riscv/qkv4.hpp"
#endif

#endif

#ifdef __cplusplus 
extern "C"{
#endif

    __interrupt void NMI_Handler(void);
    __interrupt void HardFault_Handler(void);

#ifdef __cplusplus
}
#endif

#endif