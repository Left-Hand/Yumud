#pragma once

#include <functional>

#include "core/sdk.hpp"

#include "hal/bus/bus_base.hpp"
#include "uart_utils.hpp"

#include "core/container/ringbuf.hpp"


namespace ymd::hal{

#ifndef UART_FIFO_BUF_SIZE
static constexpr size_t UART_FIFO_BUF_SIZE = 256;
#endif


#ifndef UART_DMA_BUF_SIZE
static constexpr size_t UART_DMA_BUF_SIZE = 64;
#endif

#ifndef UART_TX_DMA_BUF_SIZE
static constexpr size_t UART_TX_DMA_BUF_SIZE = UART_DMA_BUF_SIZE;
#endif


#ifndef UART_RX_DMA_BUF_SIZE
static constexpr size_t UART_RX_DMA_BUF_SIZE = UART_DMA_BUF_SIZE;
#endif


class Uart{

public:
    using Mode = CommDirection;
    using Callback = std::function<void(void)>;
    using Parity = UartParity;

public:
    hal::HalResult read(uint32_t & data) {
        char _;read1(_);data = _;return hal::HalResult::Ok();};

    hal::HalResult write(const uint32_t data) {
        write1(char(data)); return hal::HalResult::Ok();};

    virtual void writeN(const char * pdata, const size_t len) = 0;

    virtual void write1(const char data) = 0;

    void read1(char & data);
    void readN(char * pbuf, const size_t len);
    Uart(const Uart & other) = delete;
    Uart(Uart && other) = delete;

    struct Config{
        uint32_t baudrate;
        CommStrategy rx_strategy = CommStrategy::Dma;
        CommStrategy tx_strategy = CommStrategy::Dma;
    };

    virtual void init(const Config & cfg) = 0;

    size_t available() const {return rx_fifo_.available();}
    size_t pending() const {return tx_fifo_.available();}
    size_t remain() const {return tx_fifo_.size() - tx_fifo_.available();}

    virtual void set_tx_strategy(const CommStrategy _tx_strategy) = 0;
    virtual void set_rx_strategy(const CommStrategy _rx_strategy) = 0;
    void bind_post_tx_cb(auto && cb){post_tx_cb_ = std::move(cb);}
    void bind_post_rx_cb(auto && cb){post_rx_cb_ = std::move(cb);}


private:
    Callback post_tx_cb_;
    Callback post_rx_cb_;

protected:
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;


    RingBuf<char, UART_FIFO_BUF_SIZE> tx_fifo_;
    RingBuf<char, UART_FIFO_BUF_SIZE> rx_fifo_;

    Uart(){;}

    __fast_inline void invoke_post_tx_callback(){EXECUTE(post_tx_cb_);}
    __fast_inline void invoke_post_rx_callback(){EXECUTE(post_rx_cb_);}
};



}