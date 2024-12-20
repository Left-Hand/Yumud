#pragma once

//#region sdk
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

#ifdef N32G45X

#define HDW_SXX32
#include "sdk/n32g45x/CMSIS/core/core_cm4.h"
#include "sdk/n32g45x/CMSIS/device/n32g45x.h"
#endif


#ifdef USE_CH32V3_STD_SDK
#include "sdk/ch32v307/Peripheral/inc/ch32v30x_conf.h"
#include "sdk/ch32v307/Peripheral/inc/ch32v30x.h"
#include "sdk/ch32v307/system_ch32v30x.h"
#include "sdk/ch32v307/Core/core_riscv.h"
#endif

#ifdef USE_CH32V2_STD_SDK
#include "sdk/ch32v203/Peripheral/inc/ch32v20x_conf.h"
#include "sdk/ch32v203/system_ch32v20x.h"
#include "sdk/ch32v203/Core/core_riscv.h"
#endif
//#endregion