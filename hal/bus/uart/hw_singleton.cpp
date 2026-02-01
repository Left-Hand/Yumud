#include "hw_singleton.hpp"
#include "uart.hpp"
#include "core/sdk.hpp"


namespace ymd::hal{
#ifdef USART1_PRESENT
Uart usart1{USART1};
#endif

#ifdef USART2_PRESENT
Uart usart2{USART2};
#endif

#ifdef USART3_PRESENT
Uart usart3{USART3};
#endif

#ifdef UART4_PRESENT
Uart uart4{UART4};
#endif

#ifdef UART5_PRESENT
Uart uart5{UART5};
#endif

#ifdef UART6_PRESENT
Uart uart6{UART6};
#endif

#ifdef UART7_PRESENT
Uart uart7{UART7};
#endif

#ifdef UART8_PRESENT
Uart uart8{UART8};
#endif
}


extern "C"{

#ifdef USART1_PRESENT
__interrupt void USART1_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::usart1);
}
#endif

#ifdef USART2_PRESENT
__interrupt void USART2_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::usart2);
}
#endif

#ifdef USART3_PRESENT
__interrupt void USART3_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::usart3);
}
#endif

#ifdef UART4_PRESENT
__interrupt void UART4_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::uart4);
}
#endif

#ifdef UART5_PRESENT
__interrupt void UART5_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::uart5);
}
#endif

#ifdef UART6_PRESENT
__interrupt void UART6_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::uart6);
}
#endif

#ifdef UART7_PRESENT
__interrupt void UART7_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::uart7);
}
#endif

#ifdef UART8_PRESENT
__interrupt void UART8_IRQHandler(){
    ymd::hal::UartInterruptDispatcher::on_interrupt(ymd::hal::uart8);
}
#endif
}