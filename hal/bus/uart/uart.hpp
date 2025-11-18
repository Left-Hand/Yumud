#pragma once

#include <functional>

#include "core/sdk.hpp"

#include "hal/bus/bus_enums.hpp"
#include "primitive/hal_result.hpp"

#include "uart_primitive.hpp"

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
    using Event = UartEvent;
    using Parity = UartParity;



    using Callback = std::function<void(Event)>;

    struct Config{
        uint32_t baudrate;
        CommStrategy rx_strategy = CommStrategy::Dma;
        CommStrategy tx_strategy = CommStrategy::Dma;
    };

    Uart(const Uart & other) = delete;
    Uart(Uart && other) = delete;

    virtual void init(const Config & cfg) = 0;

    [[nodiscard]] size_t available() const {return rx_fifo_.available();}
    [[nodiscard]] size_t pending() const {return tx_fifo_.available();}
    [[nodiscard]] size_t remain() const {return tx_fifo_.capacity() - tx_fifo_.available();}

    virtual void set_tx_strategy(const CommStrategy tx_strategy) = 0;
    virtual void set_rx_strategy(const CommStrategy rx_strategy) = 0;

    template<typename Fn>
    void set_event_handler(Fn && cb){callback_ = std::forward<Fn>(cb);}

    HalResult read(uint32_t & data) {
        char _;read1(_);data = _;return HalResult::Ok();};

    HalResult write(const uint32_t data) {
        write1(char(data)); return HalResult::Ok();};

    virtual void writeN(const char * pdata, const size_t len) = 0;

    virtual void write1(const char data) = 0;

    void read1(char & data);
    void readN(char * pbuf, const size_t len);

    auto & tx_fifo(){return tx_fifo_;}
    auto & rx_fifo(){return rx_fifo_;}
private:
    Callback callback_;


protected:
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;


    RingBuf<char, UART_FIFO_BUF_SIZE> tx_fifo_;
    RingBuf<char, UART_FIFO_BUF_SIZE> rx_fifo_;
    void invoke_callback(const Event event){
        if(callback_ == nullptr) return;
        callback_(event);
    }
    Uart(){;}

};



}