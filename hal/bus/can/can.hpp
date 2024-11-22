#pragma once

#include "hal/bus/bus.hpp"

#include "CanUtils.hpp"
#include "CanMsg.hpp"

#include "CanTrait.hpp"
// #include "interrupts.hpp"


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

    Fifo_t<CanMsg, CAN_SOFTFIFO_SIZE> pending_rx_msgs;
    Callback cb_txok = nullptr;
    Callback cb_txfail = nullptr;
    Callback cb_rx = nullptr;

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


    void init(const BaudRate baudRate, const Mode mode, const CanFilter & filter);
    friend class CanFilter;
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}
    void setBaudRate(const uint32_t baudRate) override;

    void init(const uint baudRate, const Mode mode = Mode::Normal);

    bool write(const CanMsg & msg) override;
    CanMsg read() override;
    const CanMsg & front();
    size_t pending();
    size_t available();

    void clear(){while(this->available()){this->read();}}

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