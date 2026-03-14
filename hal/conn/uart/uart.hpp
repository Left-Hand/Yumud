#pragma once

#include "uart_base.hpp"
#include "uart_lld.hpp"
#include "hal/dma/dma_primitive.hpp"


namespace ymd::hal{

class DmaChannel;

struct Uart;
struct UartIrqHandler{
    static void on_interrupt(Uart & self);

    static void isr_txe(Uart & self);
    static void isr_tc(Uart & self);

    static void isr_rxne(Uart & self);
    static void isr_idle(Uart & self);
};

class Uart final:public UartBase{
public:
    explicit Uart(
        void * p_inst
    );

    void init(const Config & cfg);
    void deinit();

    void enable_single_line_mode(const Enable en);

    [[nodiscard]] size_t try_write_bytes(std::span<const uint8_t> bytes);

    [[nodiscard]] size_t try_write_byte(const uint8_t byte);

    void set_tx_strategy(const CommStrategy tx_strategy);

    void set_rx_strategy(const CommStrategy rx_strategy);
private:
    void * p_inst_;
    Nth inst_nth_;

    void enable_rcc(const Enable en);
    void register_nvic(hal::NvicPriorityCode priority, const Enable en);
    void set_remap(const UartRemap remap);

    void enable_tx(const Enable en);
    void enable_rx(const Enable en);

    void enable_rxne_interrupt(const Enable en);
    void enable_idle_interrupt(const Enable en);
    void enable_tx_interrupt(const Enable en);

    void enable_tx_dma(const Enable en);
    void enable_rx_dma(const Enable en);
    void setup_rx_dma(const DmaPriority priority);
    void setup_tx_dma(const DmaPriority priority);

    void poll_tx_dma();



    std::array<uint8_t, UART_TX_DMA_BUF_SIZE> tx_dma_buf_;
    std::array<uint8_t, UART_RX_DMA_BUF_SIZE> rx_dma_buf_;

    DmaChannel & tx_dma_;
    DmaChannel & rx_dma_;

public:
    volatile size_t tx_dma_buf_index_ = 0;
    volatile size_t rx_dma_buf_index_ = 0;

    friend class UartIrqHandler;
};



}
