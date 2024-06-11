#ifndef __UART_HPP__

#define __UART_HPP__

#include "../bus.hpp"
#include "sys/kernel/stream.hpp"

#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"

#include <functional>

class Uart:public IOStream{
public:
    using Mode = CommMode;
    using Callback = std::function<void(void)>;

    CommMethod txMethod = CommMethod::None;
    CommMethod rxMethod = CommMethod::None;

protected:

    static constexpr size_t uart_fifo_size = 256;

    RingBuf_t<char, uart_fifo_size> txBuf;
    RingBuf_t<char, uart_fifo_size> rxBuf;

public:
    void read(char & data) override;
    void read(char * data_ptr, const size_t len) override;

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