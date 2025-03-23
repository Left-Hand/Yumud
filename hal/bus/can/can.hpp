#pragma once

#include "hal/bus/bus_base.hpp"
#include "core/buffer/ringbuf/Fifo_t.hpp"
#include "core/sdk.hpp"

#include "can_utils.hpp"
#include "can_msg.hpp"

#include "can_trait.hpp"
#include "can_filter.hpp"


#ifdef ENABLE_CAN1
extern "C"{
__interrupt
void USB_HP_CAN1_TX_IRQHandler(void);

__interrupt
void USB_LP_CAN1_RX0_IRQHandler(void);

__interrupt
void CAN1_RX1_IRQHandler(void);

__interrupt
void CAN1_SCE_IRQHandler(void);
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

__interrupt
void CAN2_SCE_IRQHandler(void);
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
protected:
    CAN_TypeDef * instance;
    
    #ifndef CAN_SOFTFIFO_SIZE
    #define CAN_SOFTFIFO_SIZE 8
    #endif

    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> rx_fifo_;
    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> tx_fifo_;

    Callback cb_txok_;
    Callback cb_txfail_;
    Callback cb_rx_;

    bool sync_ = true;

    Gpio & get_tx_gpio();
    Gpio & get_rx_gpio();
    BusError lead(const uint8_t index){return BusError::OK;};
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


    void init(const BaudRate baudrate, const Mode mode);
    uint8_t transmit(const CanMsg & msg);
    CanMsg receive(const uint8_t fifo_num);

    friend class CanFilter;

    #ifdef ENABLE_CAN1
    friend void ::USB_HP_CAN1_TX_IRQHandler(void);

    friend void ::USB_LP_CAN1_RX0_IRQHandler(void);

    friend void ::CAN1_RX1_IRQHandler(void);

    friend void ::CAN1_SCE_IRQHandler(void);
    #endif

    #ifdef ENABLE_CAN2
    friend void ::CAN2_TX_IRQHandler(void);

    friend void ::CAN2_RX0_IRQHandler(void);

    friend void ::CAN2_RX1_IRQHandler(void);

    friend void ::CAN2_SCE_IRQHandler(void);
    #endif
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}
    Can(const Can & other) = delete;
    Can(Can && other) = delete;

    void set_baudrate(const uint32_t baudrate);

    void init(const uint baudrate, const Mode mode = Mode::Normal);

    bool write(const CanMsg & msg);
    const CanMsg && read();
    const CanMsg & front();
    size_t pending();
    size_t available();

    void clear(){while(this->available()){this->read();}}
    void set_sync(const bool en){sync_ = en;}
    bool is_tranmitting();
    bool is_receiving();
    void enable_hw_retransmit(const bool en = true);
    void cancel_transmit(const uint8_t mbox);
    void cancel_all_transmits();
    void enable_fifo_lock(const bool en = true);
    void enable_index_priority(const bool en = true);
    uint8_t get_tx_errcnt();
    uint8_t get_rx_errcnt();
    CanError error();

    bool is_busoff();

    void bind_tx_ok_cb(auto && cb){cb_txok_ = std::forward<decltype(cb)>(cb);}
    void bind_tx_fail_cb(auto && cb){cb_txfail_ = std::forward<decltype(cb)>(cb);}
    void bind_rx_cb(auto && cb){cb_rx_ = std::forward<decltype(cb)>(cb);}

    CanFilter operator[](const size_t idx) const ;
};

#ifdef ENABLE_CAN1
extern Can can1;
#endif

#ifdef ENABLE_CAN2
extern Can can2;
#endif

}