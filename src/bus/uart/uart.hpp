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


    CommMethod txMethod = CommMethod::None;
    CommMethod rxMethod = CommMethod::None;

protected:
    Mode mode = Mode::TxRx;

    void _read(char & data) override;
    void _read(char * data_ptr, const size_t len) override;


public:
    RingBuf_t<char, 128> rxBuf;
    RingBuf_t<char, 128> txBuf;

    virtual void init(
        const uint32_t & baudRate, 
        const Mode & _mode = Mode::TxRx, 
        const CommMethod & _rxMethod = CommMethod::Interrupt,
        const CommMethod & _txMethod = CommMethod::Blocking) = 0;
    size_t available(){return rxBuf.available();}

    virtual void flush(){}

    virtual void setTxMethod(const CommMethod & _txMethod) = 0;

    virtual void setRxMethod(const CommMethod & _rxMethod) = 0;
};
#endif