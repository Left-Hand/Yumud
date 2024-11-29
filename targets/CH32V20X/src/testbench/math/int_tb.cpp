#include "../tb.h"

#include "sys/math/int/int_t.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "sys/debug/debug_inc.h"

void int_tb() {
    DEBUGGER_INST.init(DEBUG_UART_BAUD);

    // i8 a = {0};

    // DEBUG_PRINTLN(a);    
}