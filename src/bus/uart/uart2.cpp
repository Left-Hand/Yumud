#include "uart2.hpp"

#ifdef HAVE_UART2
Uart2 uart2;

__interrupt
void USART2_IRQHandler(){
    if(USART_GetITStatus(USART2,USART_IT_RXNE)){
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
        uart2.rxBuf.addData(USART_ReceiveData(USART2));
    }else if(USART_GetITStatus(USART2,USART_IT_TXE)){
        USART_ClearITPendingBit(USART2,USART_IT_TXE);
        if(uart2.txBuf.available()){
            USART2->DATAR = uart2.txBuf.getData();
        }else{
            USART_ITConfig(USART2, USART_IT_TXE, false);
        }
    }else if(USART_GetFlagStatus(USART2,USART_FLAG_ORE))
    {
        USART_ClearFlag(USART2,USART_FLAG_ORE);
        USART_ReceiveData(USART2);
    }
}

#endif