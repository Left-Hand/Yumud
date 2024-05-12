#ifndef __UART1_HW_HPP__

#define __UART1_HW_HPP__

#include "uarthw.hpp"

class Uart1 : public UartHw{
public:
    Uart1():UartHw(USART1){;}

    void enableTxDma(const bool & en = true) override {if(en) setupDma((char *)&USART1->DATAR, &txBuf[0], txBuf.getSize(), DMA1_Channel4, false);}
    void enableRxDma(const bool & en = true) override {if(en) setupDma((char *)&USART1->DATAR, &rxBuf[0], rxBuf.getSize(), DMA1_Channel5, true);}
};


#ifdef HAVE_UART1
extern Uart1 uart1;

extern "C" {
__interrupt void USART1_IRQHandler();
}

#endif

#endif