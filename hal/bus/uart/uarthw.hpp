#pragma once

#include "uart.hpp"


#ifdef ENABLE_UART1
extern "C" __interrupt void USART1_IRQHandler(void);
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

class DmaChannel;

class UartHw final:public Uart{
public:
protected:
    USART_TypeDef * instance_;

    void enable_rcc(const Enable en);
    void enable_it(const Enable en);
    void enable_rxne_it(const Enable en);
    void enable_idle_it(const Enable en);
    void invoke_tx_it();

    void enable_rx_dma(const Enable en);
    void enable_tx_dma(const Enable en);

    void on_rx_dma_done();
    void on_rx_dma_half();

    void invoke_tx_dma();

    __fast_inline void on_rxne_interrupt(){
        this->rx_fifo_.push(uint8_t(instance_->DATAR));
    }
    
    __fast_inline void on_txe_interrupt(){
    
    }
    
    void on_rxidle_interrupt();

    size_t rx_dma_buf_index_;


    std::array<char, UART_DMA_BUF_SIZE> tx_dma_buf_;
    std::array<char, UART_DMA_BUF_SIZE> rx_dma_buf_;

    DmaChannel & tx_dma_;
    DmaChannel & rx_dma_;

    hal::HalResult lead(const LockRequest req) override;
    void trail() override;
public:
    UartHw(USART_TypeDef * instance, DmaChannel & tx_dma, DmaChannel & rx_dma):
            instance_(instance), tx_dma_(tx_dma), rx_dma_(rx_dma){;}

    void init(
        const uint32_t baudrate, 
        const CommStrategy rx_strategy = CommStrategy::Dma,
        const CommStrategy tx_strategy = CommStrategy::Dma
    );

    void enable_single_line_mode(const Enable en = EN);

    void writeN(const char * data_ptr, const size_t len) override;

    void write1(const char data) override;
    
    void set_tx_strategy(const CommStrategy tx_strategy);

    void set_rx_strategy(const CommStrategy rx_strategy);

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



