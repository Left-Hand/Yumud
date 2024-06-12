#ifndef __CAN_HPP__
#define __CAN_HPP__

#include <memory>
#include <functional>
#include "src/platform.h"
#include "hal/gpio/gpio.hpp"
#include "can_msg.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "hal/bus/bus_inc.h"
#include "can_filter.hpp"


// class Can{
// public:
//     enum class BaudRate{
//         Kbps125,
//         Mbps1
//     };

//     using Callback = std::function<void(void)>;
// protected:
//     CAN_TypeDef * instance;

//     virtual void settleTxPin(const uint8_t & remap) = 0;
//     virtual void settleRxPin(const uint8_t & remap) = 0;
// public:
//     Can(CAN_TypeDef * _instance):instance(_instance){;}
//     virtual void init(const BaudRate & baudRate, const uint8_t & remap = 0, const CanFilter & filter = CanFilter()) = 0;
//     virtual bool write(const CanMsg & msg) = 0;
//     virtual size_t pending() = 0;
//     virtual const CanMsg & read() = 0;
//     virtual size_t available() = 0;

//     virtual bool isTranmitting() = 0;
//     virtual bool isReceiving() = 0;
//     virtual void enableHwReTransmit(const bool en = true) = 0;
//     virtual void cancelTransmit(const uint8_t mbox) = 0;
//     virtual void cancelAllTransmit() = 0;

//     virtual void enableFifoLock(const bool en = true) = 0;
//     virtual void enableIndexPriority(const bool en = true) = 0;
//     virtual uint8_t getTxErrCnt() = 0;
//     virtual uint8_t getRxErrCnt() = 0;
//     virtual uint8_t getErrCode() = 0;

//     virtual bool isBusOff() = 0;

//     virtual void bindCbTxOk(const Callback & _cb) = 0;
//     virtual void bindCbTxFail(const Callback & _cb) = 0;
//     virtual void bindCbRx(const Callback & _cb) = 0;
// };



class Can{
public:
    enum class BaudRate{
        Kbps125,
        Mbps1
    };

    using Callback = std::function<void(void)>;
protected:
    CAN_TypeDef * instance;
    void settleTxPin(const uint8_t & remap);
    void settleRxPin(const uint8_t & remap);
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}
    void init(const BaudRate & baudRate, const uint8_t & remap = 0, const CanFilter & filter = CanFilter());
    bool write(const CanMsg & msg);
    size_t pending();
    const CanMsg & read();
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
    uint8_t getErrCode();

    bool isBusOff();

    void bindCbTxOk(const Callback & _cb);
    void bindCbTxFail(const Callback & _cb);
    void bindCbRx(const Callback & _cb);
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