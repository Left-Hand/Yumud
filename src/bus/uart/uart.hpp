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
    Mode mode = Mode::TxRx;

    void _read(char & data) override;
    void _read(char * data_ptr, const size_t len) override;


public:
    RingBuf<128> ringBuf;

    size_t available(){return ringBuf.available();}
};

class UartHw:public Uart{
protected:
    USART_TypeDef * instance;

    Gpio getTxPin();
    Gpio getRxPin();

    void enableRcc(const bool en = true);
    void enableRxIt(const bool en = true);
    void _write(const char * data_ptr, const size_t & len);

    void _write(const char & data);

public:
    UartHw(USART_TypeDef * _instance):instance(_instance){;}

    void init(const uint32_t & baudRate, const Mode _mode = Mode::TxRx);

    void setBaudRate(const uint32_t & baudRate){init(baudRate);}

};
#endif