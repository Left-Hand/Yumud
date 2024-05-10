#ifndef __UARTHW_HPP__
#define __UARTHW_HPP__

#include "uart.hpp"

class UartHw:public Uart{
public:
protected:
    USART_TypeDef * instance;

    Gpio & getTxPin();
    Gpio & getRxPin();

    void enableRcc(const bool & en = true);

    void _write(const char * data_ptr, const size_t & len);

    void _write(const char & data);

    void setupDma(char * reg_ptr, char * buf_ptr, size_t buf_size, DMA_Channel_TypeDef * dma_instance, const bool & buf_as_receiver);

    void setupNvic(const bool & en = true);


    CommMethod txMethod = CommMethod::None;
    CommMethod rxMethod = CommMethod::None;

    void triggerTxIt(){
        if(txMethod == CommMethod::Interrupt){
            USART_ITConfig(instance, USART_IT_TXE, true);
        }
    }

    void enableRxIt(const bool &en = true);

    void enableTxIt(const bool &en = true);

    virtual void enableTxDma(const bool &en = true) = 0;

    virtual void enableRxDma(const bool &en = true) = 0;
public:


    UartHw(USART_TypeDef * _instance):instance(_instance){;}

    void init(
        const uint32_t & baudRate, 
        const Mode & _mode = Mode::TxRx, 
        const CommMethod & _rxMethod = CommMethod::Interrupt,
        const CommMethod & _txMethod = CommMethod::Blocking);

    void setTxMethod(const CommMethod & _txMethod);

    void setRxMethod(const CommMethod & _rxMethod);
};

#endif /* UARTHW_HPP */
