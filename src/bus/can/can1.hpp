#ifndef __CAN_HPP__
#define __CAN_HPP__

#include <memory>
#include "src/platform.h"
#include "can_msg.hpp"
#include "types/buffer/ringbuf/ringbuf_t.hpp"

class Can{
public:
    enum class BaudRate{
        Kbps125,
        Mbps1
    };

};


class Can1:public Can{
public:
    void init(const BaudRate & baudRate);
    bool write(const CanMsg & msg);
    size_t pending();
    CanMsg & read();
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