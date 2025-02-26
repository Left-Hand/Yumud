#include "../tb.h"

#include "sys/math/int/int_t.hpp"
#include "sys/debug/debug.hpp"

#include "hal/bus/uart/uarthw.hpp"


void int_tb() {
    DEBUGGER_INST.init(DEBUG_UART_BAUD);

    // i8 a = {0};

    // DEBUG_PRINTLN(a);    
}