#ifndef __CAN_HPP__
#define __CAN_HPP__

#include <memory>
#include <functional>
#include "src/platform.h"
#include "src/gpio/gpio.hpp"
#include "can_msg.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "src/bus/bus_inc.h"
#include "can_filter.hpp"


class Can{
public:
    enum class BaudRate{
        Kbps125,
        Mbps1
    };

    using Callback = std::function<void(void)>;
protected:
    virtual void settleTxPin(const uint8_t & remap) = 0;
    virtual void settleRxPin(const uint8_t & remap) = 0;
public:
    virtual void init(const BaudRate & baudRate, const uint8_t & remap = 0, const CanFilter & filter = CanFilter()) = 0;
    virtual bool write(const CanMsg & msg) = 0;
    virtual size_t pending() = 0;
    virtual const CanMsg & read() = 0;
    virtual size_t available() = 0;

    virtual bool isTranmitting() = 0;
    virtual bool isReceiving() = 0;
    virtual void enableHwReTransmit(const bool en = true) = 0;
    virtual void cancelTransmit(const uint8_t mbox) = 0;
    virtual void cancelAllTransmit() = 0;

    virtual void enableFifoLock(const bool en = true) = 0;
    virtual void enableIndexPriority(const bool en = true) = 0;
    virtual uint8_t getTxErrCnt() = 0;
    virtual uint8_t getRxErrCnt() = 0;
    virtual uint8_t getErrCode() = 0;

    virtual bool isBusOff() = 0;

    virtual void bindCbTxOk(const Callback & _cb) = 0;
    virtual void bindCbTxFail(const Callback & _cb) = 0;
    virtual void bindCbRx(const Callback & _cb) = 0;
};



class Can1:public Can{
protected:
    void settleTxPin(const uint8_t & remap) override;
    void settleRxPin(const uint8_t & remap) override;
public:
    void init(const BaudRate & baudRate, const uint8_t & remap = 0, const CanFilter & filter = CanFilter()) override;
    bool write(const CanMsg & msg) override;
    size_t pending() override;
    const CanMsg & read() override;
    size_t available() override;

    bool isTranmitting() override;
    bool isReceiving() override;
    void enableHwReTransmit(const bool en = true) override;
    void cancelTransmit(const uint8_t mbox) override;
    void cancelAllTransmit() override;
    void enableFifoLock(const bool en = true) override;
    void enableIndexPriority(const bool en = true) override;
    uint8_t getTxErrCnt() override;
    uint8_t getRxErrCnt() override;
    uint8_t getErrCode() override;

    bool isBusOff() override;

    void bindCbTxOk(const Callback & _cb) override;
    void bindCbTxFail(const Callback & _cb) override;
    void bindCbRx(const Callback & _cb) override;
};

extern Can1 can1;
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