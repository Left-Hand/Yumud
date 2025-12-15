#pragma once

#include "uart.hpp"


#ifdef UART1_PRESENT
extern "C" __interrupt void USART1_IRQHandler(void);
#endif

#ifdef UART2_PRESENT
extern "C" __interrupt void USART2_IRQHandler(void);
#endif

#ifdef UART3_PRESENT
extern "C" __interrupt void USART3_IRQHandler(void);
#endif

#ifdef UART4_PRESENT
extern "C" __interrupt void UART4_IRQHandler(void);
#endif

#ifdef UART5_PRESENT
extern "C" __interrupt void UART5_IRQHandler(void);
#endif

#ifdef UART6_PRESENT
extern "C" __interrupt void UART6_IRQHandler(void);
#endif

#ifdef UART7_PRESENT
extern "C" __interrupt void UART7_IRQHandler(void);
#endif

#ifdef UART8_PRESENT
extern "C" __interrupt void UART8_IRQHandler(void);
#endif


namespace ymd::hal{

class DmaChannel;

class UartHw final:public Uart{
public:
    explicit UartHw(
        void * inst
    );

    void init(const Config & cfg);

    void enable_single_line_mode(const Enable en);

    [[nodiscard]] size_t try_write_chars(const char * data_ptr, const size_t len);

    [[nodiscard]] size_t try_write_char(const char data);
    
    void set_tx_strategy(const CommStrategy tx_strategy);

    void set_rx_strategy(const CommStrategy rx_strategy);

private:
    void * inst_;

    void enable_rcc(const Enable en);
    void register_nvic(const Enable en);
    void set_remap(const UartRemap remap);

    void enable_rxne_it(const Enable en);
    void enable_idle_it(const Enable en);
    void enable_tx_it(const Enable en);

    void enable_rx_dma(const Enable en);
    void enable_tx_dma(const Enable en);

    void invoke_tx_dma();

    void on_rxne_interrupt();
    
    void on_txe_interrupt();
    
    void on_rxidle_interrupt();


    std::array<char, UART_DMA_BUF_SIZE> tx_dma_buf_;
    std::array<char, UART_DMA_BUF_SIZE> rx_dma_buf_;

    DmaChannel & tx_dma_;
    DmaChannel & rx_dma_;

    #ifdef UART1_PRESENT
    friend void ::USART1_IRQHandler();
    #endif

    #ifdef UART2_PRESENT
    friend void ::USART2_IRQHandler(void);
    #endif

    #ifdef UART3_PRESENT
    friend void ::USART3_IRQHandler(void);
    #endif

    #ifdef UART4_PRESENT
    friend void ::UART4_IRQHandler(void);
    #endif

    #ifdef UART5_PRESENT
    friend void ::UART5_IRQHandler(void);
    #endif

    #ifdef UART6_PRESENT
    friend void ::UART6_IRQHandler(void);
    #endif

    #ifdef UART7_PRESENT
    friend void ::UART7_IRQHandler(void);
    #endif

    #ifdef UART8_PRESENT
    friend void ::UART8_IRQHandler(void);
    #endif
public:
    size_t rx_dma_buf_index_ = 0;
};


#ifdef UART1_PRESENT
extern UartHw uart1;
#endif

#ifdef UART2_PRESENT
extern UartHw uart2;
#endif

#ifdef UART3_PRESENT
extern UartHw uart3;
#endif

#ifdef UART4_PRESENT
extern UartHw uart4;
#endif

#ifdef UART5_PRESENT
extern UartHw uart5;
#endif

#ifdef UART6_PRESENT
extern UartHw uart6;
#endif

#ifdef UART7_PRESENT
extern UartHw uart7;
#endif

#ifdef UART8_PRESENT
extern UartHw uart8;
#endif

}



