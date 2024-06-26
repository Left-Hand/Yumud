#ifndef __CAN_HPP__
#define __CAN_HPP__

#include <memory>
#include <functional>
#include "sys/platform.h"
#include "hal/gpio/gpio.hpp"
#include "can_msg.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"
#include "hal/bus/bus_inc.h"
#include "can_filter.hpp"


class Can: public PackedBus<CanMsg>{
public:
    enum class BaudRate{
        Kbps125,
        Kbps250,
        Kbps500,
        Mbps1
    };

    using Packet = CanMsg;
    using Callback = std::function<void(void)>;
protected:
    CAN_TypeDef * instance;

    Gpio & getTxGpio();
    Gpio & getRxGpio();
    Error lead(const uint8_t index) override{return ErrorType::OK;};
    void trail() override{};
public:
    Can(CAN_TypeDef * _instance):instance(_instance){;}
    void configBaudRate(const uint32_t baudRate) override;
    void init(const BaudRate baudRate, const CanFilter & filter = CanFilter());
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
    uint8_t getErrCode();

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