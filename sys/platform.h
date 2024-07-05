#ifndef __PLATFORM_H__

#define __PLATFORM_H__

#include "defines/defs.h"

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


#define HIGH true
#define LOW false

#endif