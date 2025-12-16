#pragma once

#include "core/utils/Result.hpp"
#include "core/container/ringbuf.hpp"
#include "ral/can.hpp"
#include "can_layout.hpp"

#include "primitive/can/bxcan_frame.hpp"
#include "primitive/can/can_event.hpp"

#include "can_filter.hpp"



//#region switches

// #define CAN_SCE_ENABLED

//#endregion switches

#ifdef CAN1_PRESENT
extern "C"{
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void);

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void);

__interrupt
void CAN1_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN1_SCE_IRQHandler(void);
#endif
}
#endif

#ifdef CAN2_PRESENT
extern "C"{
__interrupt
void CAN2_TX_IRQHandler(void);

__interrupt
void CAN2_RX0_IRQHandler(void);

__interrupt
void CAN2_RX1_IRQHandler(void);

#ifdef CAN_SCE_ENABLED
__interrupt
void CAN2_SCE_IRQHandler(void);
#endif
}
#endif

namespace ymd::hal{

class Gpio;

struct CanFilter;

struct Can;

struct CanInterruptDispatcher{
    static void on_tx_interrupt(Can & can);
    static void on_rx_interrupt(Can & can, const CanFifoIndex fifo_idx);
    static void on_sce_interrupt(Can & can);
};

class [[nodiscard]] Can final{
public:
    using BaudRate = CanBaudrate;
    using WiringMode = CanWiringMode;
    using Exception = CanException;
    using Error = CanLibError;

    using Callback = std::function<void(CanEvent)>;


    struct [[nodiscard]] Config{
        CanRemap remap;
        WiringMode wiring_mode;
        CanNominalBitTimming bit_timming;
    };

public:
    explicit Can(void * inst):inst_(inst){;}
    Can(const Can & other) = delete;
    Can(Can && other) = delete;

    void init(const Config & cfg);
    
    [[nodiscard]] uint32_t get_aligned_bus_clk_freq();

    [[nodiscard]] Result<void, CanLibError> try_write(const BxCanFrame & frame);
    [[nodiscard]] BxCanFrame read();
    [[nodiscard]] size_t free_capacity(){return tx_fifo_.free_capacity();}
    [[nodiscard]] size_t available();

    [[nodiscard]] bool is_tranmitting();
    [[nodiscard]] bool is_receiving();

    void enable_hw_retransmit(const Enable en);
    void cancel_transmit(const CanMailboxIndex mailbox_index);
    void cancel_all_transmits();
    void enable_fifo_lock(const Enable en);
    void enable_index_priority(const Enable en);

    [[nodiscard]] uint8_t get_tx_errcnt();
    [[nodiscard]] uint8_t get_rx_errcnt();
    [[nodiscard]] Option<CanException> last_exception();
    [[nodiscard]] bool is_busoff();

    template<typename Fn>
    void set_event_handler(Fn && cb){
        callback_ = std::forward<Fn>(cb);
    }


    template<size_t I>
    requires (I < 14)
    CanFilter filters() const {
        return CanFilter(this->inst_, Nth(I));
    }

private:
    void * inst_;
    
    #ifndef CAN_SOFTFIFO_SIZE
    static constexpr size_t CAN_SOFTFIFO_SIZE = 8;
    #endif

    RingBuf<BxCanFrame, CAN_SOFTFIFO_SIZE> rx_fifo_;
    RingBuf<BxCanFrame, CAN_SOFTFIFO_SIZE> tx_fifo_;

    Callback callback_ = nullptr;

    void alter_to_pins(const CanRemap remap);
    void enable_rcc(const Enable en);
    void set_remap(const CanRemap remap);

    void init_interrupts();

    void transmit(const BxCanFrame & frame, const CanMailboxIndex mbox_index);
    [[nodiscard]] BxCanFrame receive(const CanFifoIndex fifo_idx);

    friend class CanFilter;
    friend class CanInterruptDispatcher;
};

#ifdef CAN1_PRESENT
extern Can can1;
#endif

#ifdef CAN2_PRESENT
extern Can can2;
#endif

}