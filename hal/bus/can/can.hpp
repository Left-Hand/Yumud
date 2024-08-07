#ifndef __CAN_HPP__
#define __CAN_HPP__

#include "can_utils.hpp"
#include "can_msg.hpp"
#include "can_filter.hpp"
#include "../hal/bus/bus.hpp"


class Can: public PackedBus<CanMsg>{
public:
    using BaudRate = CanUtils::BaudRate;
    using Mode = CanUtils::Mode;
    using ErrCode = CanUtils::ErrCode;

    using Packet = CanMsg;
    using Callback = std::function<void(void)>;
protected:
    CAN_TypeDef * instance;

    Gpio & getTxGpio();
    Gpio & getRxGpio();
    Error lead(const uint8_t index) override{return ErrorType::OK;};
    void trail() override{};

    void installGpio();
    void enableRcc();
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}
    void configBaudRate(const uint32_t baudRate) override;

    void init(const uint baudRate, const Mode mode = Mode::Normal, const CanFilter & filter = CanFilter());

    void init(const BaudRate baudRate, const Mode mode = Mode::Normal, const CanFilter & filter = CanFilter());

    /**
     * @brief Writes a CAN message to the transmit buffer.
     *
     * This function attempts to write a CAN message to the transmit buffer.
     * If the transmit buffer is full, the function will return false.
     *
     * @param msg The CAN message to be written.
     * @return true if the CAN message was successfully written to the transmit buffer.
     * @return false if the transmit buffer is full and the CAN message could not be written.
     */
    bool write(const CanMsg & msg) override;
    const CanMsg & read() override;
    size_t pending();
    size_t available();

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
};

#ifdef HAVE_CAN1
extern Can can1;


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

#endif