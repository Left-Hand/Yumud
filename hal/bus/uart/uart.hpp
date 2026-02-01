#pragma once

#include "uart_base.hpp"


namespace ymd::hal{

class DmaChannel;

struct Uart;
struct UartInterruptDispatcher{

    static void isr_txe(Uart & self);
    static void isr_tc(Uart & self);

    static void isr_rxne(Uart & self);
    static void isr_rxidle(Uart & self);

    static void on_interrupt(Uart & uart);
};

class Uart final:public UartBase{
public:
    explicit Uart(
        void * inst
    );

    void init(const Config & cfg);
    void deinit();

    void enable_single_line_mode(const Enable en);

    [[nodiscard]] size_t try_write_bytes(std::span<const uint8_t> bytes);

    [[nodiscard]] size_t try_write_byte(const uint8_t byte);

    void set_tx_strategy(const CommStrategy tx_strategy);

    void set_rx_strategy(const CommStrategy rx_strategy);

    [[nodiscard]] size_t sink_bytes(std::span<const uint8_t> bytes);

private:
    void * inst_;

    void enable_rcc(const Enable en);
    void register_nvic(const Enable en);
    void set_remap(const UartRemap remap);

    void enable_rxne_interrupt(const Enable en);
    void enable_idle_interrupt(const Enable en);
    void enable_tx_interrupt(const Enable en);

    void enable_rx_dma(const Enable en);
    void enable_tx_dma(const Enable en);



    std::array<uint8_t, UART_DMA_BUF_SIZE> tx_dma_buf_;
    std::array<uint8_t, UART_DMA_BUF_SIZE> rx_dma_buf_;

    DmaChannel & tx_dma_;
    DmaChannel & rx_dma_;

public:
    volatile size_t tx_dma_buf_index_ = 0;
    volatile size_t rx_dma_buf_index_ = 0;

    friend class UartInterruptDispatcher;
};



}
