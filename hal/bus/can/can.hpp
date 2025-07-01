#pragma once

#include "hal/bus/bus_base.hpp"
#include "core/container/ringbuf/Fifo_t.hpp"
#include "core/sdk.hpp"

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
class Can: public BusBase{
public:
    using BaudRate = CanBaudrate;
    using Mode = CanMode;
    using ErrCode = CanError;

    using Callback = std::function<void(void)>;

public:
    Can(CAN_TypeDef * instance):inst_(instance){;}
    Can(const Can & other) = delete;
    Can(Can && other) = delete;

    void set_baudrate(const uint32_t baudrate);

    struct Config{
        BaudRate baudrate;
        Mode mode = Mode::Normal;
    };

    void init(const Config & cfg);

    bool write(const CanMsg & msg);
    [[nodiscard]] CanMsg read();
    [[nodiscard]] size_t pending();
    [[nodiscard]] size_t available();

    void clear_rx(){while(this->available()){(void)this->read();}}
    void set_sync(const Enable en){sync_ = en == EN;}
    [[nodiscard]] bool is_tranmitting();
    [[nodiscard]] bool is_receiving();
    void enable_hw_retransmit(const Enable en = EN);
    void cancel_transmit(const uint8_t mbox);
    void cancel_all_transmits();
    void enable_fifo_lock(const Enable en = EN);
    void enable_index_priority(const Enable en = EN);
    [[nodiscard]] uint8_t get_tx_errcnt();
    [[nodiscard]] uint8_t get_rx_errcnt();
    [[nodiscard]] CanError get_last_error();
    [[nodiscard]] bool is_busoff();

    void bind_tx_ok_cb(auto && cb){cb_txok_ = std::forward<decltype(cb)>(cb);}
    void bind_tx_fail_cb(auto && cb){cb_txfail_ = std::forward<decltype(cb)>(cb);}
    void bind_rx_cb(auto && cb){cb_rx_ = std::forward<decltype(cb)>(cb);}

    CanFilter operator[](const size_t idx) const ;

private:
    CAN_TypeDef * inst_;
    
    #ifndef CAN_SOFTFIFO_SIZE
    static constexpr size_t CAN_SOFTFIFO_SIZE = 8;
    #endif

    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> rx_fifo_;
    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> tx_fifo_;

    Callback cb_txok_;
    Callback cb_txfail_;
    Callback cb_rx_;

    bool sync_ = false;

    Gpio & get_tx_gpio();
    Gpio & get_rx_gpio();

    hal::HalResult lead(const hal::LockRequest req){
        return hal::HalResult::Ok();
    };

    void trail(){};

    void install_gpio();
    void enable_rcc();
    bool is_mail_box_done(const uint8_t mbox);
    void clear_mailbox(const uint8_t mbox);
    void init_it();
    
    void on_tx_interrupt();
    void on_rx_msg_interrupt(const uint8_t fifo_num);
    void on_rx_overrun_interrupt(){;}
    void on_rx_full_interrupt(){;}
    void on_sce_interrupt();



    [[nodiscard]] std::optional<uint8_t> transmit(const CanMsg & msg);
    [[nodiscard]] CanMsg receive(const uint8_t fifo_num);

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