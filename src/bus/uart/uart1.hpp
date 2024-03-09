#ifndef __UART1_HW_HPP__

#define __UART1_HW_HPP__

#include "uart.hpp"

class Uart1 : public UartHw{
public:
    Uart1():UartHw(USART1){;}
};


#ifdef HAVE_UART1
extern Uart1 uart1;

extern "C" {
__interrupt void USART1_IRQHandler();
}

#endif

#endif