#include "uart1.hpp"

#ifdef HAVE_UART1
Uart1 uart1;

__interrupt
void USART1_IRQHandler(void){
    if(USART_GetITStatus(USART1,USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
        uart1.rxBuf.addData(USART_ReceiveData(USART1));
    }else if(USART_GetITStatus(USART1,USART_IT_TXE))
    {
        USART_ClearITPendingBit(USART1,USART_IT_TXE);
        // if(uart1.txBuf.available()){
        //     USART_SendData(USART1, uart1.txBuf.getData());
        // }
    }else if(USART_GetFlagStatus(USART1,USART_FLAG_ORE))
    {
        USART_ClearFlag(USART1,USART_FLAG_ORE);
        USART_ReceiveData(USART1);
    }
}

#endif