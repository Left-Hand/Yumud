#include "tb.h"

#define UART_TB_ECHO

void uart_tb(Uart & uart){
    #ifdef UART_TB_ECHO
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