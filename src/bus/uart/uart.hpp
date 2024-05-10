#ifndef __UART_HPP__

#define __UART_HPP__

#include "src/gpio/gpio.hpp"
#include "../printer.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "src/gpio/port.hpp"

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
    RingBuf_t<char, 128> rxBuf;
    RingBuf_t<char, 128> txBuf;

    size_t available(){return rxBuf.available();}

    void flush(){}
};
#endif