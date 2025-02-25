#pragma once

#include "uart.hpp"

#include "hal/dma/dma.hpp"



#ifdef ENABLE_UART1
extern "C" __interrupt void USART1_IRQHandler();
#endif

#ifdef ENABLE_UART2
extern "C" __interrupt void USART2_IRQHandler(void);
#endif

#ifdef ENABLE_UART3
extern "C" __interrupt void USART3_IRQHandler(void);
#endif

#ifdef ENABLE_UART4
extern "C" __interrupt void UART4_IRQHandler(void);
#endif

#ifdef ENABLE_UART5
extern "C" __interrupt void UART5_IRQHandler(void);
#endif

#ifdef ENABLE_UART6
extern "C" __interrupt void UART6_IRQHandler(void);
#endif

#ifdef ENABLE_UART7
extern "C" __interrupt void UART7_IRQHandler(void);
#endif

#ifdef ENABLE_UART8
extern "C" __interrupt void UART8_IRQHandler(void);
#endif


namespace ymd::hal{


class UartHw:public Uart{
public:
protected:
    USART_TypeDef * instance;

    void enableRcc(const bool en);
    void enableIt(const bool en);
    void enableRxneIt(const bool en);
    void enableIdleIt(const bool en);
    void invokeTxIt();

    void enableRxDma(const bool en);
    void enableTxDma(const bool en);

    void rxDmaDoneHandler();
    void rxDmaHalfHandler();

    void invokeTxDma();

    void rxneHandle();
    void txeHandle();
    void idleHandle();

    size_t rx_dma_buf_index;
    size_t tx_dma_buf_index;

    #ifndef UART_DMA_BUF_SIZE
    #define UART_DMA_BUF_SIZE 64
    #endif

    std::array<char, UART_DMA_BUF_SIZE> tx_dma_buf;
    std::array<char, UART_DMA_BUF_SIZE> rx_dma_buf;

    DmaChannel & tx_dma;
    DmaChannel & rx_dma;

    Error lead(const uint8_t _address) override;
    void trail() override;
public:
    void write(const char * data_ptr, const size_t len) override;

    void write(const char data) override;

    UartHw(USART_TypeDef * _instance, DmaChannel & _tx_dma, DmaChannel & _rx_dma):
            instance(_instance), tx_dma(_tx_dma), rx_dma(_rx_dma){;}

    void init(
        const uint32_t baudRate, 
        const CommMethod _txMethod = CommMethod::Dma,
        const CommMethod _rxMethod = CommMethod::Dma) override;

    void setTxMethod(const CommMethod _txMethod) override;

    void setRxMethod(const CommMethod _rxMethod) override;

    void setBaudRate(const uint32_t baudRate) override{;}//TODO
    void setParity(const Parity parity) override{;};//TODO

    Gpio & txio() override;
    Gpio & rxio() override;

    #ifdef ENABLE_UART1
    friend void ::USART1_IRQHandler();
    #endif

    #ifdef ENABLE_UART2
    friend void ::USART2_IRQHandler(void);
    #endif

    #ifdef ENABLE_UART3
    friend void ::USART3_IRQHandler(void);
    #endif

    #ifdef ENABLE_UART4
    friend void ::UART4_IRQHandler(void);
    #endif

    #ifdef ENABLE_UART5
    friend void ::UART5_IRQHandler(void);
    #endif

    #ifdef ENABLE_UART6
    friend void ::UART6_IRQHandler(void);
    #endif

    #ifdef ENABLE_UART7
    friend void ::UART7_IRQHandler(void);
    #endif

    #ifdef ENABLE_UART8
    friend void ::UART8_IRQHandler(void);
    #endif

};


#ifdef ENABLE_UART1
extern UartHw uart1;
#endif

#ifdef ENABLE_UART2
extern UartHw uart2;
#endif

#ifdef ENABLE_UART3
extern UartHw uart3;
#endif

#ifdef ENABLE_UART4
extern UartHw uart4;
#endif

#ifdef ENABLE_UART5
extern UartHw uart5;
#endif

#ifdef ENABLE_UART6
extern UartHw uart6;
#endif

#ifdef ENABLE_UART7
extern UartHw uart7;
#endif

#ifdef ENABLE_UART8
extern UartHw uart8;
#endif

}



