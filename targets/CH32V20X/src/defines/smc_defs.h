#pragma once

#include "../sys/core/platform.h"

#define F_CPU 144000000
#define USE_IQ

#define IQ_USE_LOG

#define EXTRA_IQ

#define HAVE_UART4
#define HAVE_UART7
#define UART4_REMAP 3
#define UART7_REMAP 2

#define HAVE_SPI2
#define SPI2_REMAP 1

#define HAVE_TIM1
#define TIM1_REMAP 3


#define HAVE_TIM4
#define TIM4_REMAP 1

#define HAVE_TIM5
#define TIM5_REMAP 0


#define HAVE_TIM8
#define TIM8_REMAP 0



#define HAVE_USBFS

#define DEBUGGER uart7
#define LOGGER uart4