#ifndef __UARTS_HPP__

#define __UARTS_HPP__

#include "uarthw.hpp"

#ifdef HAVE_UART1
extern UartHw uart1;

// extern "C" __interrupt void USART1_IRQHandler();

#endif

#ifdef HAVE_UART2

extern UartHw uart2;

extern "C" __interrupt void USART2_IRQHandler(void);

#endif

#endif