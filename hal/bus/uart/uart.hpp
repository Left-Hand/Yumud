#pragma once

#include <functional>

#include "sys/core/sdk.h"
#include "sys/stream/stream.hpp"

#include "hal/bus/bus.hpp"
#include "hal/gpio/port.hpp"

#include "types/buffer/ringbuf/Fifo_t.hpp"


namespace ymd::hal{

enum class UartParity{
    None = USART_Parity_No,
    Even = USART_Parity_Even,
    Odd = USART_Parity_Odd
};

class Uart:public FullDuplexBus{

public:
    using Mode = CommMode;
    using Callback = std::function<void(void)>;
    using Parity = UartParity;

private:
    Callback posttx_cb_;
    Callback postrx_cb_;

protected:
    CommMethod tx_method_;
    CommMethod rx_method_;

    #ifndef UART_FIFO_BUF_SIZE
    #define UART_FIFO_BUF_SIZE 256
    #endif

    Fifo_t<char, UART_FIFO_BUF_SIZE> tx_fifo;
    Fifo_t<char, UART_FIFO_BUF_SIZE> rx_fifo;

    Uart(){;}

    void callPostTxCallback(){EXECUTE(posttx_cb_);}
    void callPostRxCallback(){EXECUTE(postrx_cb_);}
public:
    BusError read(uint32_t & data) override {char _;read1(_);data = _;return BusError::OK;};
    BusError write(const uint32_t data) override {write1((char)data); return BusError::OK;};

    BusError transfer(uint32_t & data_rx, const uint32_t data_tx) override {write1((char)data_tx); return BusError::OK;};

    virtual void writeN(const char * data_ptr, const size_t len) = 0;

    virtual void write1(const char data) = 0;

    void read1(char & data);
    void readN(char * pdata, const size_t len);
    Uart(const Uart & other) = delete;
    Uart(Uart && other) = delete;


    virtual Gpio & txio() = 0;
    virtual Gpio & rxio() = 0;

    virtual void init(
        const uint32_t baudRate, 
        const CommMethod _rxMethod = CommMethod::Interrupt,
        const CommMethod _txMethod = CommMethod::Blocking) = 0;

    size_t available() const {return rx_fifo.available();}
    size_t pending() const {return tx_fifo.available();}
    size_t remain() const {return tx_fifo.size() - tx_fifo.available();}

    virtual void setTxMethod(const CommMethod _txMethod) = 0;
    virtual void setRxMethod(const CommMethod _rxMethod) = 0;
    void bindPostTxCb(auto && cb){posttx_cb_ = std::move(cb);}
    void bindPosRxCb(auto && cb){postrx_cb_ = std::move(cb);}
};



}