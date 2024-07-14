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

[[maybe_unused]] static void uart_tb(Uart & uart){
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
        delay(300);
        tx_led = false;
        uart.println("nothing", uart7.available());
        tx_led = true;
    }
    #endif
}

[[maybe_unused]] static void uart_tb_old(){

    USART_InitTypeDef USART_InitStructure = {0};
    // UART7_TX_GPIO.outpp();
    UART7_TX_GPIO.afpp();
    UART7_RX_GPIO.inflt();

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);

    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, ENABLE);


    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(UART7, &USART_InitStructure);
    
    USART_Cmd(UART7, ENABLE);
    [[maybe_unused]] auto write = [](const char * str){
        for(size_t i=0;str[i];i++){
            UART7->DATAR = (uint8_t)str[i];
            while((UART7->STATR & USART_FLAG_TXE) == RESET);
        }
    };
    
    while(true){
        write("hello\r\n");
        // UART7_TX_GPIO.set();
        // delay(200);
        // UART7_TX_GPIO.clr();
        // delay(200);
    }


}
void uart_main(){
    uart_tb_old();
    // auto & logger = uart7;
    // // uart7.init(115200);
    // logger.init(115200, CommMethod::Blocking, CommMethod::Interrupt);
    // uart_tb(logger);
}