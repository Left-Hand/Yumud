#include "uart1.hpp"

#ifdef HAVE_UART1
Uart1 uart1;

__interrupt
void USART1_IRQHandler(){
    if(USART_GetITStatus(USART1,USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
        uart1.ringBuf.addData(USART_ReceiveData(USART1));
    }
    if(USART_GetFlagStatus(USART1,USART_FLAG_ORE))
    {
        USART_ClearFlag(USART1,USART_FLAG_ORE);
        USART_ReceiveData(USART1);
    }
}

#endif