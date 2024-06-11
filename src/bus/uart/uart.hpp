#ifndef __UART_HPP__

#define __UART_HPP__

#include "src/gpio/gpio.hpp"
#include "../printer.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "src/gpio/port.hpp"
#include <functional>

class Uart:public Printer{
public:
    using Mode = CommMode;
    using Callback = std::function<void(void)>;

    CommMethod txMethod = CommMethod::None;
    CommMethod rxMethod = CommMethod::None;

protected:

    void _read(char & data) override;
    void _read(char * data_ptr, const size_t len) override;

    static constexpr size_t uart_fifo_size = 256;

    RingBuf_t<char, uart_fifo_size> txBuf;
    RingBuf_t<char, uart_fifo_size> rxBuf;

public:

    virtual void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) = 0;
    size_t available() const {return rxBuf.available();}
    size_t pending() const {return txBuf.available();}
    virtual void flush(){}

    virtual void setTxMethod(const CommMethod _txMethod) = 0;

    virtual void setRxMethod(const CommMethod _rxMethod) = 0;
};
#endif