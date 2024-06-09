#ifndef __UARTHW_HPP__
#define __UARTHW_HPP__

#include "uart.hpp"
#include "src/dma/dma.hpp"

class UartHw:public Uart{
public:
protected:
    USART_TypeDef * instance;

    Gpio & getTxPin();
    Gpio & getRxPin();

    void enableRcc(const bool en = true);

    void _write(const char * data_ptr, const size_t & len) override;

    void _write(const char & data) override;

    void enableIt(const bool en = true);


    void enableRxIt(const bool en = true);
    void enableTxIt(const bool en = true);
    void enableIdleIt(const bool en = true);
    void invokeTxIt();

    void enableRxDma(const bool en = true);
    void enableTxDma(const bool en = true);


public:
    void invokeTxDma();

    UartHw(USART_TypeDef * _instance):instance(_instance){;}

    void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) override;

    void setTxMethod(const CommMethod _txMethod) override;

    void setRxMethod(const CommMethod _rxMethod) override;
};

#endif /* UARTHW_HPP */
