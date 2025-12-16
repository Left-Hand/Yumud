#include "src/testbench/tb.h"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "core/clock/clock.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;


#define UART_TB_ECHO

using namespace ymd;
[[maybe_unused]] static void uart_tb(hal::Uart & uart){
    #ifdef UART_TB_ECHO

    auto tx_led = hal::PC<13>();
    auto rx_led = hal::PC<14>();
    tx_led.outpp();
    rx_led.outpp();

    uart.set_event_handler([&](const hal::UartEvent& ev){
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                rx_led.set_high();
                clock::delay(1ms);
                rx_led.set_low();
                break;
            case hal::UartEvent::TxIdle:
                tx_led.set_high();
                clock::delay(1ms);
                tx_led.set_low();
                break;
            default:
                PANIC{"Unexpected event", ev};
                break;
        }
    });

    while(true){
        // size_t size = uart.available();
        while(uart.available()){
            char chr;
            const auto len = uart.try_read_char(chr);
            if(len)
            (void)uart.try_write_char(chr);
            clock::delay(1ms);
        }
        clock::delay(300ms);
        tx_led.set_low();
        DEBUG_PRINTLN("noth", uart.available());
        tx_led.set_high();
    }
    #endif
}

void uart_main(){
    // uart_tb_old();
    //usart1 passed
    //usart2 passed
    //usart3 passed
    //uart5 passed
    //uart6 passed
    //uart8 passed

    hal::usart2.init({
        .remap = hal::UART2_REMAP_PA2_PA3,
        .baudrate = 576000
    });
    DEBUGGER.retarget(&hal::usart2);
    // DEBUGGER.init(DEBUG_UART_BAUD, CommStrategy::Dma, CommStrategy::None);
    uart_tb(hal::usart2);
}