#pragma once

#include "../bus.hpp"

#include "../types/buffer/ringbuf/ringbuf_t.hpp"
#include "../hal/gpio/gpio.hpp"
#include "../hal/gpio/port.hpp"

#include <functional>

class Uart:public IOStream, DuplexBus{
public:
    using Mode = CommMode;
    using Callback = std::function<void(void)>;
    using DuplexBus::Error;
    using DuplexBus::ErrorType;
    using DuplexBus::txMethod;
    using DuplexBus::rxMethod;
    using InputStream::read;

    enum class Parity{
        None = USART_Parity_No,
        Even = USART_Parity_Even,
        Odd = USART_Parity_Odd
    };

protected:

    #ifndef UART_FIFO_BUF_SIZE
    #define UART_FIFO_BUF_SIZE 256
    #endif

    RingBuf_t<char, UART_FIFO_BUF_SIZE> txBuf;
    RingBuf_t<char, UART_FIFO_BUF_SIZE> rxBuf;

    Callback txPostCb;
    Callback rxPostCb;

    Error read(uint32_t & data, const bool toack) override {char _;read(_);return ErrorType::OK;};
    Error write(const uint32_t data) override {write((char)data); return ErrorType::OK;};
public:
    void read(char & data) override;
    void read(char * data_ptr, const size_t len) override;

    virtual Gpio & txio() = 0;
    virtual Gpio & rxio() = 0;
    virtual void write(const char * data_ptr, const size_t len) = 0;
    virtual void write(const char data) = 0;
    virtual void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) = 0;

    size_t available() const {return rxBuf.available();}
    size_t pending() const {return txBuf.available();}
    size_t remain() const {return txBuf.size - txBuf.available();}
    virtual void flush(){}//TODO
    virtual void setTxMethod(const CommMethod _txMethod) = 0;
    virtual void setRxMethod(const CommMethod _rxMethod) = 0;
    virtual void setParity(const Parity parity) = 0;
    void bindTxPostCb(Callback && cb){txPostCb = cb;}

    void bindRxPostCb(Callback && cb){rxPostCb = cb;}
};
