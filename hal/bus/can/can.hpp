#ifndef __CAN_HPP__
#define __CAN_HPP__

#include "can_utils.hpp"
#include "can_msg.hpp"
#include "can_filter.hpp"
#include "../hal/bus/bus.hpp"

#ifdef HAVE_CAN1
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

#ifdef HAVE_CAN2
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

#ifndef CAN_FIFO_SIZE
#define CAN_FIFO_SIZE 8
#endif

class Can: public PackedBus<CanMsg>{
public:
    using BaudRate = CanUtils::BaudRate;
    using Mode = CanUtils::Mode;
    using ErrCode = CanUtils::ErrCode;

    using Packet = CanMsg;
    using Callback = std::function<void(void)>;
protected:
    CAN_TypeDef * instance;

    RingBuf_t<CanMsg, CAN_FIFO_SIZE> pending_rx_msgs;
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
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}
    void configBaudRate(const uint32_t baudRate) override;

    void init(const uint baudRate, const Mode mode = Mode::Normal, const CanFilter & filter = CanFilter());

    void init(const BaudRate baudRate, const Mode mode = Mode::Normal, const CanFilter & filter = CanFilter());

    bool write(const CanMsg & msg) override;
    const CanMsg & read() override;
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


    #ifdef HAVE_CAN1
    friend void USB_HP_CAN1_TX_IRQHandler(void);

    friend void USB_LP_CAN1_RX0_IRQHandler(void);

    friend void CAN1_RX1_IRQHandler(void);

    friend void CAN1_SCE_IRQHandler(void);
    #endif

    #ifdef HAVE_CAN2
    friend void CAN2_TX_IRQHandler(void);

    friend void CAN2_RX0_IRQHandler(void);

    friend void CAN2_RX1_IRQHandler(void);

    friend void CAN2_SCE_IRQHandler(void);
    #endif
    };

#ifdef HAVE_CAN1
extern Can can1;
#endif

#ifdef HAVE_CAN2
extern Can can2;
#endif

#endif