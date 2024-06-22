#include "tb.h"

#define UART_TB_ECHO

void uart_tb(Uart & uart){
    #ifdef UART_TB_ECHO
    while(true){
        if(uart.available()){
            delay(5);
            uart.println(uart.readString());
        }
        delay(300);
        uart.println("nothing");
    }
    #endif
}