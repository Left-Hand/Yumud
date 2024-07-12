#ifndef __PLATFORM_H__

#define __PLATFORM_H__

#include "defines/defs.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


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
#include <ch32v20x.h>
#include <ch32v20x_opa.h>
#endif

#ifdef CH32V30X
#include "sdk/ch32v307/Peripheral/inc/ch32v30x_conf.h"
#include "sdk/ch32v307/Peripheral/inc/ch32v30x.h"
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

#if defined(__cplusplus)
#include "../sys/kernel/enums.hpp"

#include <type_traits>
#include <limits>

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;


__fast_inline uint64_t operator"" _KHz(uint64_t x){
    return x * 1000;
}

__fast_inline uint64_t operator"" _MHz(uint64_t x){
    return x * 1000000;
}

__fast_inline uint64_t operator"" _GHz(uint64_t x){
    return x * 1000000000;
}

__fast_inline uint64_t operator"" _KB(uint64_t x){
    return x << 10;
}

__fast_inline uint64_t operator"" _MB(uint64_t x){
    return x << 20;
}

__fast_inline uint64_t operator"" _GB(uint64_t x){
    return x << 30;
}

#endif

#endif