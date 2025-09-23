#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"



using namespace ymd;


#define UART hal::uart2
// #define OTHER_UART hal::uart1
#define OTHER_UART hal::uart2
static constexpr size_t BAUD = 921600;

void half_line_uart_main(){

    UART.init({BAUD});
    DEBUGGER.retarget(&UART);

    OTHER_UART.init({BAUD});
    OTHER_UART.enable_single_line_mode(EN);

    while(true){
        const auto ava = OTHER_UART.available();
        DEBUG_PRINTLN(ava);
        // for(size_t i = 0; i < ava; i++){
        //     char chr;
        //     uart1.read1(chr);
        //     DEBUG_PRINT(int(chr));
        // }
        clock::delay(10ms);
    }
}