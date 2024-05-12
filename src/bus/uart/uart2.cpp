#include "uart2.hpp"

#ifdef HAVE_UART2
Uart2 uart2;

__interrupt void USART2_IRQHandler(void){
    volatile uint16_t & flag_w = USART2->STATR; 
    uint16_t flag_r;
    flag_r = flag_w;

    if(flag_r & USART_FLAG_RXNE){
        flag_w = flag_r ^ USART_FLAG_RXNE;
        uart2.rxBuf.addData(USART_ReceiveData(USART2));
    }else if(flag_r & USART_FLAG_TXE){
        flag_w = flag_r ^ USART_FLAG_TXE;
        if(uart2.txBuf.available()){
            USART2->DATAR = uart2.txBuf.getData();
        }else{
            USART_ITConfig(USART2, USART_IT_TXE, false);
        }
    }else if(flag_r & USART_FLAG_ORE){
        flag_w = flag_r ^ USART_FLAG_ORE;
        USART_ReceiveData(USART2);
    }
}


__interrupt void DMA1_Channel6_IRQHandler(void){
	if(DMA_GetFlagStatus(DMA1_FLAG_TC6)){
	
		DMA_ClearFlag(DMA1_FLAG_TC6);
	}
}
__interrupt void DMA1_Channel7_IRQHandler(void){
	if(DMA_GetFlagStatus(DMA1_FLAG_TC7)){
	
		DMA_ClearFlag(DMA1_FLAG_TC7);
	}
}
#endif