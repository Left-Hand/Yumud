#include "tb.h"

#include <string>
#include "sys/clock/clock.hpp"

#define UART_TB_ECHO
using std::string;

[[maybe_unused]] static void getline(IOStream & logger, string & str){
    String temp_str;
    while(true){
        if(logger.available()){
            char chr;
            logger.read(chr);
            if(chr == '\n'){

                if(temp_str.length()){
                    str = temp_str.c_str();
                    return;
                }
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
        }
        else{
            delay(400);
            logger.print(' ');
        }
    }
}

[[maybe_unused]] static void uart_tb(Uart & uart){
    #ifdef UART_TB_ECHO

    auto & tx_led = portC[13];
    auto & rx_led = portC[14];
    tx_led.outpp();
    rx_led.outpp();

    uart.onTxDone([&](){
        tx_led.set();
        delay(1);
        tx_led.clr();
    });

    uart.onRxDone([&](){
        rx_led.set();
        delay(1);
        rx_led.clr();
    });

    while(true){
        // size_t size = uart.available();
        // if(uart.available()){
        //     delay(1);
        //     auto str = uart.readString();
        //     str.toUpperCase();
        //     uart.println(str, size);
        // }
        delay(300);
        tx_led = false;
        uart.println("noth", uart.available());
        tx_led = true;
    }
    #endif
}

void uart_main(){
    // uart_tb_old();
    //uart1 passed
    //uart2 passed
    //uart3 passed
    //uart5 passed
    //uart6 passed
    //uart8 passed

    auto & logger = DEBUGGER_INST;
    logger.init(DEBUG_UART_BAUD, CommMethod::Dma, CommMethod::Interrupt);
    // DEBUGGER.init(DEBUG_UART_BAUD, CommMethod::Dma, CommMethod::None);
    uart_tb(logger);
}