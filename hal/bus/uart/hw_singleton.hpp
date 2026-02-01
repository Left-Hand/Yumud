#pragma once

#include "uart.hpp"


extern "C"{

#ifdef USART1_PRESENT
__interrupt void USART1_IRQHandler(void);
#endif

#ifdef USART2_PRESENT
__interrupt void USART2_IRQHandler(void);
#endif

#ifdef USART3_PRESENT
__interrupt void USART3_IRQHandler(void);
#endif

#ifdef UART4_PRESENT
__interrupt void UART4_IRQHandler(void);
#endif

#ifdef UART5_PRESENT
__interrupt void UART5_IRQHandler(void);
#endif

#ifdef UART6_PRESENT
__interrupt void UART6_IRQHandler(void);
#endif

#ifdef UART7_PRESENT
__interrupt void UART7_IRQHandler(void);
#endif

#ifdef UART8_PRESENT
__interrupt void UART8_IRQHandler(void);
#endif

}

namespace ymd::hal{

#ifdef USART1_PRESENT
extern Uart usart1;
#endif

#ifdef USART2_PRESENT
extern Uart usart2;
#endif

#ifdef USART3_PRESENT
extern Uart usart3;
#endif

#ifdef UART4_PRESENT
extern Uart uart4;
#endif

#ifdef UART5_PRESENT
extern Uart uart5;
#endif

#ifdef UART6_PRESENT
extern Uart uart6;
#endif

#ifdef UART7_PRESENT
extern Uart uart7;
#endif

#ifdef UART8_PRESENT
extern Uart uart8;
#endif
}