#ifndef __DEBUG_INC_H__

#define __DEBUG_INC_H__

#include "bus/bus_inc.h"
#include "clock/clock.h"


#ifndef DEBUG_LOG
#define DEBUG_LOG(...) uart2.println(__VA_ARGS__)
#endif

#endif