#ifndef __UART2_HW_HPP__

#define __UART2_HW_HPP__

#include "uarthw.hpp"

class Uart2 : public UartHw{
public:
    Uart2():UartHw(USART2){;}

    void enableTxDma(const bool & en = true) override {
        if(en){
            setupDma((char *)&USART1->DATAR, &txBuf[0], txBuf.getSize(), DMA1_Channel7, false);
        }
        USART_DMACmd(USART2, USART_DMAReq_Tx , en);
    }
    void enableRxDma(const bool & en = true) override {
        if(en){
            // usartdm
        }
        USART_DMACmd(USART2, USART_DMAReq_Rx , en);
    }
};


#ifdef HAVE_UART2
extern Uart2 uart2;

extern "C" {
__interrupt void USART2_IRQHandler(void);
__interrupt void DMA1_Channel6_IRQHandler(void);
__interrupt void DMA1_Channel7_IRQHandler(void);

}

#endif

#endif