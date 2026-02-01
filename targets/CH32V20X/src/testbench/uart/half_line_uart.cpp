#include "src/testbench/tb.h"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/hw_singleton.hpp"



using namespace ymd;


#define UART hal::usart2
// #define OTHER_UART hal::usart1
#define OTHER_UART hal::usart2
static constexpr size_t BAUD = 921600;

void half_line_uart_main(){

    UART.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(BAUD)
    });
    DEBUGGER.retarget(&UART);

    OTHER_UART.init({
        .remap = hal::UartRemap::_0,
        .baudrate = hal::NearestFreq(BAUD)
    });
    OTHER_UART.enable_single_line_mode(EN);

    while(true){
        const auto ava = OTHER_UART.available();
        DEBUG_PRINTLN(ava);
        // for(size_t i = 0; i < ava; i++){
        //     char chr;
        //     usart1.read_char(chr);
        //     DEBUG_PRINT(int(chr));
        // }
        clock::delay(10ms);
    }
}