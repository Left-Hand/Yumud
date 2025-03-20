#pragma once

#include <functional>

#include "sys/core/sdk.h"
#include "sys/stream/stream.hpp"

#include "hal/bus/bus.hpp"
#include "hal/gpio/port.hpp"

#include "sys/buffer/ringbuf/Fifo_t.hpp"


namespace ymd::hal{

enum class UartParity{
    None = USART_Parity_No,
    Even = USART_Parity_Even,
    Odd = USART_Parity_Odd
};

class Uart:public FullDuplexBus{

public:
    using Mode = CommDirection;
    using Callback = std::function<void(void)>;
    using Parity = UartParity;

private:
    Callback posttx_cb_;
    Callback postrx_cb_;

protected:
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;

    #ifndef UART_FIFO_BUF_SIZE
    #define UART_FIFO_BUF_SIZE 256
    #endif

    Fifo_t<char, UART_FIFO_BUF_SIZE> tx_fifo_;
    Fifo_t<char, UART_FIFO_BUF_SIZE> rx_fifo_;

    Uart(){;}

    void callPostTxCallback(){EXECUTE(posttx_cb_);}
    void callPostRxCallback(){EXECUTE(postrx_cb_);}
public:
    BusError read(uint32_t & data) override {char _;read1(_);data = _;return BusError::OK;};
    BusError write(const uint32_t data) override {write1(char(data)); return BusError::OK;};

    BusError transfer(uint32_t & data_rx, const uint32_t data_tx) override {write1(char(data_tx)); return BusError::OK;};

    virtual void writeN(const char * data_ptr, const size_t len) = 0;

    virtual void write1(const char data) = 0;

    void read1(char & data);
    void readN(char * pdata, const size_t len);
    Uart(const Uart & other) = delete;
    Uart(Uart && other) = delete;


    virtual Gpio & txio() = 0;
    virtual Gpio & rxio() = 0;

    virtual void init(
        const uint32_t baudrate, 
        const CommStrategy rx_strategy = CommStrategy::Interrupt,
        const CommStrategy tx_strategy = CommStrategy::Blocking) = 0;

    size_t available() const {return rx_fifo_.available();}
    size_t pending() const {return tx_fifo_.available();}
    size_t remain() const {return tx_fifo_.size() - tx_fifo_.available();}

    virtual void set_tx_strategy(const CommStrategy _tx_strategy) = 0;
    virtual void set_rx_strategy(const CommStrategy _rxMethod) = 0;
    void bindPostTxCb(auto && cb){posttx_cb_ = std::move(cb);}
    void bindPosRxCb(auto && cb){postrx_cb_ = std::move(cb);}
};



}