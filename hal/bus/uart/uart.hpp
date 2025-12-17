#pragma once

#include <functional>

#include "core/container/ringbuf.hpp"

#include "hal/bus/bus_enums.hpp"
#include "primitive/hal_result.hpp"

#include "uart_primitive.hpp"

#include "uart_layout.hpp"


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


class UartBase{

public:
    using Mode = CommDirection;
    using Event = UartEvent;
    using Parity = UartParity;



    using Callback = std::function<void(Event)>;

    struct Config{
        UartRemap remap;
        UartBaudrate baudrate;
        CommStrategy rx_strategy = CommStrategy::Dma;
        CommStrategy tx_strategy = CommStrategy::Dma;
    };

    UartBase(const UartBase & other) = delete;
    UartBase(UartBase && other) = delete;

    virtual void init(const Config & cfg) = 0;

    [[nodiscard]] __fast_inline size_t available() const {return rx_fifo_.length();}
    [[nodiscard]] __fast_inline size_t free_capacity() const {return tx_fifo_.free_capacity();}

    virtual void set_tx_strategy(const CommStrategy tx_strategy) = 0;
    virtual void set_rx_strategy(const CommStrategy rx_strategy) = 0;

    template<typename Fn>
    void set_event_handler(Fn && cb){callback_ = std::forward<Fn>(cb);}

    [[nodiscard]] virtual size_t try_write_chars(const char * pdata, const size_t len) = 0;

    [[nodiscard]] virtual size_t try_write_char(const char data) = 0;

    [[nodiscard]] size_t try_read_char(char & data);
    [[nodiscard]] size_t try_read_chars(char * pbuf, const size_t len);

    auto & tx_fifo(){return tx_fifo_;}
    auto & rx_fifo(){return rx_fifo_;}
private:
    Callback callback_;


protected:
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;


    RingBuf<char, UART_FIFO_BUF_SIZE> tx_fifo_;
    RingBuf<char, UART_FIFO_BUF_SIZE> rx_fifo_;
    void emit_event(const Event event){
        if(callback_ == nullptr) [[unlikely]]
            return;
        callback_(event);
    }
    UartBase(){;}

};



}