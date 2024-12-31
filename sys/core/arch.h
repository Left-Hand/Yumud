#pragma once

//#region arch
#ifdef __cplusplus 


#ifdef CH32V20X
#define ARCH_QKV4
#endif

#ifdef CH32V30X
#define ARCH_QKV4
#endif

#ifdef ARCH_QKV4
#include "arch/riscv/qkv4.hpp"
#endif
#endif
//#endregion


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
#define HDW_SXX32
#ifndef USE_CH32V2_STD_SDK
#define  USE_CH32V2_STD_SDK
#endif
#endif

#ifdef CH32V30X
#define HDW_SXX32
#ifndef USE_CH32V3_STD_SDK
#define  USE_CH32V3_STD_SDK
#endif
#endif

#ifdef STM32F4XX
#define HDW_SXX32
#endif