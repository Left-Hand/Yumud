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

namespace ymd::hal{

struct CanFilter;
class Can: public PackedBus<CanMsg>{
public:
    using BaudRate = CanUtils::BaudRate;
    using Mode = CanUtils::Mode;
    using ErrCode = CanUtils::ErrCode;
    using RemoteType = CanUtils::RemoteType;

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
    Error lead(const uint8_t index) override{return Error::OK;};
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
extern Can can1;
#endif

#ifdef ENABLE_CAN2
extern Can can2;
#endif

}