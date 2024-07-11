#include "tb.h"

#define UART_TB_ECHO

static void uart_tb(Uart & uart){
    #ifdef UART_TB_ECHO

    auto & tx_led = portC[13];
    auto & rx_led = portC[14];
    tx_led.outpp();
    rx_led.outpp();
    uart.bindTxPostCb([&](){
        tx_led.set();
        delay(1);
        tx_led.clr();
    });

    uart.bindRxPostCb([&](){
        rx_led.set();
        delay(1);
        rx_led.clr();
    });

    while(true){
        size_t size = uart.available();
        if(uart.available()){
            delay(5);

            auto str = uart.readString();
            str.toUpperCase();
            uart.println(str, size);
        }
        delay(300);
        uart.println("nothing", size);
    }
    #endif
}

void uart_main(){
    Uart & uart = uart1;
    uart1.init(115200 * 8);
    uart_tb(uart);
}