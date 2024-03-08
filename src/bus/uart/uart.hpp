#ifndef __UART_HPP__

#define __UART_HPP__

#include "src/gpio/gpio.hpp"
#include "../printer.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"


class Uart:public Printer{
public:
    enum Mode:uint8_t{
        RxOnly = 1, TxOnly, TxRx = TxOnly | RxOnly
    };
protected:
    Gpio tx_pin;
    Gpio rx_pin;
    Mode mode;
public:
    RingBuf ringBuf;

    Uart(Gpio _tx_pin, Gpio _rx_pin):Printer(ringBuf),tx_pin(_tx_pin), rx_pin(_rx_pin),
            mode(Mode((tx_pin.isValid() ? (uint8_t)TxOnly : 0) | (rx_pin.isValid() ? (uint8_t)RxOnly : 0))){;}
};

class UartHw:public Uart{
protected:
    USART_TypeDef * instance;

    Gpio getTxPin(USART_TypeDef * _instance, const Mode mode);
    Gpio getRxPin(USART_TypeDef * _instance, const Mode mode);

    void _write(const char * data_ptr, const size_t & len){
        for(size_t i=0;i<len;i++) _write(data_ptr[i]);
	}

    __fast_inline void _write(const char & data){
        while((instance->STATR & USART_FLAG_TXE) == RESET);
        instance->DATAR = data;
        while((instance->STATR & USART_FLAG_TC) == RESET);
    }

public:
    UartHw(USART_TypeDef * _instance, const Mode _mode):Uart(getTxPin(_instance,_mode), getRxPin(_instance, _mode)), instance(_instance){;}

    void init(const uint32_t & baudRate);

    void setBaudRate(const uint32_t & baudRate){init(baudRate);}
    void initRxIt();


};
#endif