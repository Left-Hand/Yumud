#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/stream/stream.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"
#include "core/string/string.hpp"

using namespace ymd;
using namespace ymd::hal;

#define UART_TB_ECHO

#define TARG_UART hal::uart2

using namespace ymd;
[[maybe_unused]] static void uart_tb(Uart & uart){
    #ifdef UART_TB_ECHO

    auto & tx_led = portC[13];
    auto & rx_led = portC[14];
    tx_led.outpp();
    rx_led.outpp();

    uart.bind_post_tx_cb([&](){
        tx_led.set();
        clock::delay(1ms);
        tx_led.clr();
    });

    uart.bind_post_rx_cb([&](){
        rx_led.set();
        clock::delay(1ms);
        rx_led.clr();
    });

    while(true){
        // size_t size = uart.available();
        while(uart.available()){
            char chr;
            uart.read1(chr);
            uart.write1(chr);
            clock::delay(1ms);
        }
        clock::delay(300ms);
        tx_led.clr();
        DEBUG_PRINTLN("noth", uart.available());
        tx_led.set();
    }
    #endif
}

void uart_main(){
    // uart_tb_old();
    //uart1 passed
    //TARG_UART passed
    //uart3 passed
    //uart5 passed
    //uart6 passed
    //uart8 passed

    TARG_UART.init({
        576000, CommStrategy::Dma, 
        CommStrategy::Interrupt});
    DEBUGGER.retarget(&TARG_UART);
    // DEBUGGER.init(DEBUG_UART_BAUD, CommStrategy::Dma, CommStrategy::None);
    uart_tb(TARG_UART);
}