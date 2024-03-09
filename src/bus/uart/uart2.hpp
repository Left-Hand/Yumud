#ifndef __UART2_HW_HPP__

#define __UART2_HW_HPP__

#include "uart.hpp"

class Uart2 : public UartHw{
public:
    Uart2():UartHw(USART2){;}
};


#ifdef HAVE_UART2
extern Uart2 uart2;

extern "C" {
__interrupt void USART2_IRQHandler();
}

#endif

#endif