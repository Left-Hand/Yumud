#ifndef __UARTS_HPP__

#define __UARTS_HPP__

#include "uarthw.hpp"

#ifdef HAVE_UART1
extern UartHw uart1;

extern "C" {
    __interrupt void USART1_IRQHandler();
    __interrupt void DMA1_Channel5_IRQHandler(void);
    __interrupt void DMA1_Channel4_IRQHandler(void);
}

#endif

#ifdef HAVE_UART2

extern UartHw uart2;

extern "C"{
    __interrupt void USART2_IRQHandler(void);
    __interrupt void DMA1_Channel6_IRQHandler(void);
    __interrupt void DMA1_Channel7_IRQHandler(void);
}

#endif

#endif