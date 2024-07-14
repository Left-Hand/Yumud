#include "tb.h"

#include <string>

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

static void uart_tb(Uart & uart){
    #ifdef UART_TB_ECHO

    auto & tx_led = portC[13];
    auto & rx_led = portC[14];
    tx_led.outpp();
    rx_led.outpp();
    // uart.bindTxPostCb([&](){
    //     tx_led.set();
    //     delay(1);
    //     tx_led.clr();
    // });

    // uart.bindRxPostCb([&](){
    //     rx_led.set();
    //     delay(1);
    //     rx_led.clr();
    // });

    while(true){
        size_t size = uart.available();
        if(uart.available()){
            delay(5);

            auto str = uart.readString();
            str.toUpperCase();
            uart.println(str, size);
        }
        // delay(300);
        tx_led = false;
        uart.println("nothing", uart7.available());
        tx_led = true;
    }
    #endif
}

void uart_main(){
    auto & logger = uart2;
    logger.init(115200, CommMethod::Blocking, CommMethod::Interrupt);
    uart_tb(logger);
}