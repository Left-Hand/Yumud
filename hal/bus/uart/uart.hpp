#ifndef __UART_HPP__

#define __UART_HPP__

#include "../bus.hpp"
#include "sys/kernel/stream.hpp"

#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "hal/gpio/gpio.hpp"
#include "hal/gpio/port.hpp"

#include <functional>

class Uart:public IOStream, DuplexBus{
public:
    using Mode = CommMode;
    using Callback = std::function<void(void)>;
    using DuplexBus::Error;
    using DuplexBus::ErrorType;
    using DuplexBus::txMethod;
    using DuplexBus::rxMethod;
protected:
    static constexpr size_t uart_fifo_size = 256;
    RingBuf_t<char, uart_fifo_size> txBuf;
    RingBuf_t<char, uart_fifo_size> rxBuf;

    Error read(uint32_t & data, const bool toack) override {char _;read(_);return ErrorType::OK;};
    Error write(const uint32_t data) override {write((char)data); return ErrorType::OK;};
public:
    void read(char & data) override;
    void read(char * data_ptr, const size_t len) override;
    virtual void write(const char * data_ptr, const size_t len) = 0;
    virtual void write(const char data) = 0;
    virtual void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) = 0;
    size_t available() const {return rxBuf.available();}
    size_t pending() const {return txBuf.available();}
    virtual void flush(){}//TODO

    virtual void setTxMethod(const CommMethod _txMethod) = 0;

    virtual void setRxMethod(const CommMethod _rxMethod) = 0;
};
#endif