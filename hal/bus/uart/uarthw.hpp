#pragma once

#include "uart.hpp"

extern "C"{

#ifdef UART1_PRESENT
__interrupt void USART1_IRQHandler(void);
#endif

#ifdef UART2_PRESENT
__interrupt void USART2_IRQHandler(void);
#endif

#ifdef UART3_PRESENT
__interrupt void USART3_IRQHandler(void);
#endif

#ifdef UART4_PRESENT
__interrupt void UART4_IRQHandler(void);
#endif

#ifdef UART5_PRESENT
__interrupt void UART5_IRQHandler(void);
#endif

#ifdef UART6_PRESENT
__interrupt void UART6_IRQHandler(void);
#endif

#ifdef UART7_PRESENT
__interrupt void UART7_IRQHandler(void);
#endif

#ifdef UART8_PRESENT
__interrupt void UART8_IRQHandler(void);
#endif

}


namespace ymd::hal{

class DmaChannel;

struct Uart;
struct UartInterruptDispatcher{
    static void on_interrupt(Uart & uart);
};

class Uart final:public UartBase{
public:
    explicit Uart(
        void * inst
    );

    void init(const Config & cfg);
    void deinit();

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

    void enable_rxne_interrupt(const Enable en);
    void enable_idle_interrupt(const Enable en);
    void enable_tx_interrupt(const Enable en);

    void enable_rx_dma(const Enable en);
    void enable_tx_dma(const Enable en);

    void poll_tx_dma();

    void accept_rxne_interrupt();
    
    void accept_txe_interrupt();
    
    void accept_rxidle_interrupt();


    std::array<char, UART_DMA_BUF_SIZE> tx_dma_buf_;
    std::array<char, UART_DMA_BUF_SIZE> rx_dma_buf_;

    DmaChannel & tx_dma_;
    DmaChannel & rx_dma_;

public:
    volatile size_t rx_dma_buf_index_ = 0;

    friend class UartInterruptDispatcher;
};


#ifdef USART1_PRESENT
extern Uart usart1;
#endif

#ifdef USART2_PRESENT
extern Uart usart2;
#endif

#ifdef USART3_PRESENT
extern Uart usart3;
#endif

#ifdef UART4_PRESENT
extern Uart uart4;
#endif

#ifdef UART5_PRESENT
extern Uart uart5;
#endif

#ifdef UART6_PRESENT
extern Uart uart6;
#endif

#ifdef UART7_PRESENT
extern Uart uart7;
#endif

#ifdef UART8_PRESENT
extern Uart uart8;
#endif

}



