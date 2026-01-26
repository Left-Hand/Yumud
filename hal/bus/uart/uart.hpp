#pragma once

#include <functional>

#include "core/container/ringbuf.hpp"

#include "hal/bus/bus_enums.hpp"
#include "primitive/hal_result.hpp"

#include "uart_primitive.hpp"

#include "uart_layout.hpp"


namespace ymd::hal{

#ifndef UART_BUFFERED_QUEUE_SIZE
static constexpr size_t UART_BUFFERED_QUEUE_SIZE = 256;
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



    using EventCallback = std::function<void(Event)>;

    struct Config{
        UartRemap remap;
        UartBaudrate baudrate;
        CommStrategy rx_strategy = CommStrategy::Dma;
        CommStrategy tx_strategy = CommStrategy::Dma;
    };

    UartBase(const UartBase & other) = delete;
    UartBase(UartBase && other) = delete;

    virtual void init(const Config & cfg) = 0;

    [[nodiscard]] __fast_inline size_t available() const {return rx_queue_.length();}
    [[nodiscard]] __fast_inline size_t free_capacity() const {
        switch(tx_strategy_){
            case CommStrategy::Nil: __builtin_trap();
            case CommStrategy::Blocking: return std::numeric_limits<size_t>::max();
            case CommStrategy::Dma: return tx_queue_.free_capacity();
            case CommStrategy::Interrupt: return tx_queue_.free_capacity();
        }
        __builtin_trap();
    }

    virtual void set_tx_strategy(const CommStrategy tx_strategy) = 0;
    virtual void set_rx_strategy(const CommStrategy rx_strategy) = 0;

    template<typename Fn>
    void set_event_callback(Fn && cb){event_callback_ = std::forward<Fn>(cb);}

    [[nodiscard]] virtual size_t try_write_bytes(std::span<const uint8_t> bytes) = 0;

    [[nodiscard]] virtual size_t try_write_byte(const uint8_t byte) = 0;

    [[nodiscard]] size_t try_read_bytes(std::span<uint8_t> bytes);
    [[nodiscard]] size_t try_read_byte(uint8_t & byte);

    auto & tx_queue(){return tx_queue_;}
    auto & rx_queue(){return rx_queue_;}
// private:
protected:
    EventCallback event_callback_;


// protected:
public:
    CommStrategy tx_strategy_;
    CommStrategy rx_strategy_;


    RingBuf<uint8_t, UART_BUFFERED_QUEUE_SIZE> tx_queue_;
    RingBuf<uint8_t, UART_BUFFERED_QUEUE_SIZE> rx_queue_;
    UartBase(){;}

};



}
