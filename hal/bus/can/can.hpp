#pragma once

#include "hal/bus/bus.hpp"

#include "CanUtils.hpp"
#include "CanMsg.hpp"

#include "CanTrait.hpp"
#include "CanFilter.hpp"


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



namespace ymd{

struct CanFilter;
class Can: public PackedBus<CanMsg>,public CanTrait{
public:
    using BaudRate = CanUtils::BaudRate;
    using Mode = CanUtils::Mode;
    using ErrCode = CanUtils::ErrCode;

    using Packet = CanMsg;
    using Callback = std::function<void(void)>;
protected:
    CAN_TypeDef * instance;

    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> rx_fifo_;
    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> tx_fifo_;
    Callback cb_txok = nullptr;
    Callback cb_txfail = nullptr;
    Callback cb_rx = nullptr;
    bool sync_ = true;

    Gpio & getTxGpio();
    Gpio & getRxGpio();
    Error lead(const uint8_t index) override{return ErrorType::OK;};
    void trail() override{};

    void installGpio();
    void enableRcc();
    bool isMailBoxDone(const uint8_t mbox);
    void clearMailbox(const uint8_t mbox);
    void initIt();
    void handleTx();
    void handleRx(const uint8_t fifo_num);
    void handleSce();


    void init(const BaudRate baudRate, const Mode mode);
    uint8_t transmit(const CanMsg & msg);
    CanMsg receive(const uint8_t fifo_num);
    friend class CanFilter;

    // std::array<CanFilter, 14> filters = {
    //     CanFilter{this->instance, 0},
    //     CanFilter{this->instance, 1},
    //     CanFilter{this->instance, 2},
    //     CanFilter{this->instance, 3},
    //     CanFilter{this->instance, 4},
    //     CanFilter{this->instance, 5},
    //     CanFilter{this->instance, 6},
    //     CanFilter{this->instance, 7},
    //     CanFilter{this->instance, 8},
    //     CanFilter{this->instance, 9},
    //     CanFilter{this->instance, 10},
    //     CanFilter{this->instance, 11},
    //     CanFilter{this->instance, 12},
    //     CanFilter{this->instance, 13},
    // };
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}

    Can(const Can & other) = delete;
    Can(Can && other) = delete;
    void setBaudRate(const uint32_t baudRate) override;

    void init(const uint baudRate, const Mode mode = Mode::Normal);

    bool write(const CanMsg & msg) override;
    bool write(const CanMsg && msg) override;
    const CanMsg && read() override;
    const CanMsg & front();
    size_t pending();
    size_t available();

    void clear(){while(this->available()){this->read();}}
    void sync(const bool en){sync_ = en;}
    bool isTranmitting();
    bool isReceiving();
    void enableHwReTransmit(const bool en = true);
    void cancelTransmit(const uint8_t mbox);
    void cancelAllTransmit();
    void enableFifoLock(const bool en = true);
    void enableIndexPriority(const bool en = true);
    uint8_t getTxErrCnt();
    uint8_t getRxErrCnt();
    ErrCode getErrCode();

    bool isBusOff();

    void bindCbTxOk(Callback && _cb);
    void bindCbTxFail(Callback && _cb);
    void bindCbRx(Callback && _cb);

    CanFilter operator[](const size_t idx) const ;


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
};

#ifdef ENABLE_CAN1
inline Can can1{CAN1};
#endif

#ifdef ENABLE_CAN2
inline Can can2{CAN2};
#endif

}