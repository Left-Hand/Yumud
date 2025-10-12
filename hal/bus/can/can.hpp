#pragma once

#include "core/utils/Result.hpp"
#include "core/container/ringbuf.hpp"
#include "core/sdk.hpp"
#include "ral/chip.hpp"
#include "can_utils.hpp"
#include "can_msg.hpp"

#include "can_trait.hpp"
#include "can_filter.hpp"


//#region switches

// #define CAN_SCE_ENABLED

//#endregion switches

#ifdef ENABLE_CAN1
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

#ifdef ENABLE_CAN2
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



class Can final{
public:
    using BaudRate = CanBaudrate;
    using Mode = CanMode;
    using Fault = CanFault;
    using Error = CanError;

    using Callback = std::function<void(void)>;

    struct Config{
        CanBitTimmingCoeffs coeffs;
        Mode mode = Mode::Normal;
        uint8_t remap;
    };

public:
    explicit Can(CAN_TypeDef * inst):inst_(inst){;}
    Can(const Can & other) = delete;
    Can(Can && other) = delete;

    void set_baudrate(const uint32_t baudrate);


    void init(const Config & cfg);

    [[nodiscard]] Result<void, CanError> write(const CanMsg & msg);
    [[nodiscard]] CanMsg read();
    [[nodiscard]] size_t pending();
    [[nodiscard]] size_t available();

    [[nodiscard]] bool is_tranmitting();
    [[nodiscard]] bool is_receiving();
    void enable_hw_retransmit(const Enable en);
    void cancel_transmit(const CanMailboxNth mailbox_nth);
    void cancel_all_transmits();
    void enable_fifo_lock(const Enable en);
    void enable_index_priority(const Enable en);
    [[nodiscard]] uint8_t get_tx_errcnt();
    [[nodiscard]] uint8_t get_rx_errcnt();
    [[nodiscard]] Option<CanFault> last_fault();
    [[nodiscard]] bool is_busoff();

    template<CanIT I, typename Fn>
    void set_callback(Fn && cb){
        get_callback<I>() = std::forward<Fn>(cb);
    }


    template<size_t I>
    requires (I < 14)
    CanFilter filters() const {
        return CanFilter(this->inst_, Nth(I));
    }

private:
    CAN_TypeDef * inst_;
    
    #ifndef CAN_SOFTFIFO_SIZE
    static constexpr size_t CAN_SOFTFIFO_SIZE = 8;
    #endif

    RingBuf<CanMsg, CAN_SOFTFIFO_SIZE> rx_fifo_;
    RingBuf<CanMsg, CAN_SOFTFIFO_SIZE> tx_fifo_;

    Callback cb_txok_;
    Callback cb_txfail_;
    Callback cb_rx_;

    template<CanIT I>
    auto & get_callback(){
        if constexpr (I == CanIT::TME) return cb_txok_;
        // if constexpr (I == CanIT::)
    }

    Gpio get_tx_gpio(const uint8_t remap);
    Gpio get_rx_gpio(const uint8_t remap);

    void plant_gpio(const uint8_t remap);
    void enable_rcc(const Enable en);
    void set_remap(const uint8_t remap);
    bool is_mail_box_done(const CanMailboxNth mbox);
    void clear_mailbox(const CanMailboxNth mbox);
    void init_it();
    
    void on_tx_interrupt();
    void on_rx_msg_interrupt(const CanFifoNth fifo_num);
    void on_rx_overrun_interrupt(){;}
    void on_rx_full_interrupt(){;}
    void on_sce_interrupt();



    [[nodiscard]] Option<CanMailboxNth> transmit(const CanMsg & msg);
    [[nodiscard]] CanMsg receive(const CanFifoNth fifo_num);

    friend class CanFilter;

    #ifdef ENABLE_CAN1
    friend void ::USB_HP_CAN1_TX_IRQHandler(void);

    friend void ::USB_LP_CAN1_RX0_IRQHandler(void);

    friend void ::CAN1_RX1_IRQHandler(void);

    #ifdef CAN_SCE_ENABLED
    friend void ::CAN1_SCE_IRQHandler(void);
    #endif
    #endif

    #ifdef ENABLE_CAN2
    friend void ::CAN2_TX_IRQHandler(void);

    friend void ::CAN2_RX0_IRQHandler(void);

    friend void ::CAN2_RX1_IRQHandler(void);

    #ifdef CAN_SCE_ENABLED
    friend void ::CAN2_SCE_IRQHandler(void);
    #endif
    #endif
};

#ifdef ENABLE_CAN1
extern Can can1;
#endif

#ifdef ENABLE_CAN2
extern Can can2;
#endif

}