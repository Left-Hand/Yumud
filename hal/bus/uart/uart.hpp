#pragma once

#include "sys/core/sdk.h"
#include "hal/bus/bus.hpp"

#include "types/buffer/ringbuf/Fifo_t.hpp"
#include "hal/gpio/port.hpp"

#include <functional>

namespace ymd{
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

    Fifo_t<char, UART_FIFO_BUF_SIZE> tx_fifo;
    Fifo_t<char, UART_FIFO_BUF_SIZE> rx_fifo;

    Callback txPostCb;
    Callback rxPostCb;

    Error read(uint32_t & data, const bool toack) override {char _;read(_);return ErrorType::OK;};
    Error write(const uint32_t data) override {write((char)data); return ErrorType::OK;};

    // virtual void writeBytes(const char * pdata, const size_t len) = 0;

    Uart(){;}
public:
    Uart(const Uart & other) = delete;
    Uart(Uart && other) = delete;

    void read(char & data) override;
    void read(char * pdata, const size_t len) override;

    virtual Gpio & txio() = 0;
    virtual Gpio & rxio() = 0;

    using IOStream::write;

    virtual void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) = 0;

    size_t available() const {return rx_fifo.available();}
    size_t pending() const {return tx_fifo.available();}
    size_t remain() const {return tx_fifo.size() - tx_fifo.available();}
    virtual void flush();
    virtual void setTxMethod(const CommMethod _txMethod) = 0;
    virtual void setRxMethod(const CommMethod _rxMethod) = 0;
    virtual void setParity(const Parity parity) = 0;
    void onTxDone(Callback && cb){txPostCb = cb;}
    void onRxDone(Callback && cb){rxPostCb = cb;}
};

}