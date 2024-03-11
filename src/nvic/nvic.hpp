#ifndef __NVIC_HPP__

#define __NVIC_HPP__

#include "src/platform.h"


class InterruptController{
protected:
    const IRQn irq;
    const uint8_t pre;
    const uint8_t sub;
    const bool en = false;
public:
    InterruptController(const IRQn & _irq, const uint8_t & _pre, const uint8_t & _sub):
            irq(_irq), pre(_pre), sub(_sub){;}

    void enable(const bool & _en = true){
        NVIC_InitTypeDef NVIC_InitStructure = {0};
        NVIC_InitStructure.NVIC_IRQChannel = (uint8_t)irq;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = pre;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = sub;
        NVIC_InitStructure.NVIC_IRQChannelCmd = en;
        NVIC_Init(&NVIC_InitStructure);
    }
};
#endif