#pragma once

#include "uart.hpp"
#include "../hal/dma/dma.hpp"


class UartHw:public Uart{
public:
protected:
    USART_TypeDef * instance;

    Gpio & getTxPin();
    Gpio & getRxPin();

    void enableRcc(const bool en = true);

    void enableIt(const bool en = true);

    void enableRxneIt(const bool en = true);
    void enableTxeIt(const bool en = true);
    void enableIdleIt(const bool en = true);
    void invokeTxIt();

    void enableRxDma(const bool en = true);
    void enableTxDma(const bool en = true);


    void invokeTxDma();
    void invokeRxDma();

    void bindRxneCb(Callback && cb);
    void bindTxeCb(Callback && cb);
    void bindIdleCb(Callback && cb);

    size_t rx_dma_buf_index;
    size_t tx_dma_buf_index;

    static constexpr size_t rx_dma_buf_size = 64;
    static constexpr size_t tx_dma_buf_size = 64;

    char tx_dma_buf[tx_dma_buf_size];
    char rx_dma_buf[rx_dma_buf_size];

    DmaChannel & txDma;
    DmaChannel & rxDma;

    Error lead(const uint8_t _address) override;
    void trail() override;
public:
    void write(const char * data_ptr, const size_t len) override;

    void write(const char data) override;

    UartHw(USART_TypeDef * _instance, DmaChannel & _txDma, DmaChannel & _rxDma):
            instance(_instance), txDma(_txDma), rxDma(_rxDma){;}


    void init(
        const uint32_t baudRate, 
        const CommMethod _txMethod = CommMethod::Dma,
        const CommMethod _rxMethod = CommMethod::Dma) override;

    void setTxMethod(const CommMethod _txMethod) override;

    void setRxMethod(const CommMethod _rxMethod) override;

    void configBaudRate(const uint32_t baudRate) override{;}//TODO


};


#ifdef HAVE_UART1
extern UartHw uart1;
extern "C" __interrupt void USART1_IRQHandler();
#endif

#ifdef HAVE_UART2
extern UartHw uart2;
extern "C" __interrupt void USART2_IRQHandler(void);
#endif

#ifdef HAVE_UART3
extern UartHw uart3;
extern "C" __interrupt void USART3_IRQHandler(void);
#endif

#ifdef HAVE_UART4
extern UartHw uart4;
extern "C" __interrupt void UART4_IRQHandler(void);
#endif

#ifdef HAVE_UART5
extern UartHw uart5;
extern "C" __interrupt void UART5_IRQHandler(void);
#endif

#ifdef HAVE_UART6
extern UartHw uart6;
extern "C" __interrupt void USART6_IRQHandler(void);
#endif

#ifdef HAVE_UART7
extern UartHw uart7;
extern "C" __interrupt void UART7_IRQHandler(void);
#endif

#ifdef HAVE_UART8
extern UartHw uart8;
extern "C" __interrupt void UART8_IRQHandler(void);
#endif
