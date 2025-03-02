#include "nvic.hpp"

using namespace ymd::hal;

// static uint8_t NVIC_Priority_Group;

NvicPriority::NvicPriority(const uint8_t pre, const uint8_t sub):
    _pre(pre & 0b1), 
    _sub(sub & 0b111){;}

void NvicPriority::enable(const NvicPriority & request, const IRQn _irq, const bool en){
    // const NVIC_InitTypeDef NVIC_InitStructure = {
    //     .NVIC_IRQChannel = _irq,
    //     .NVIC_IRQChannelPreemptionPriority = request._pre,
    //     .NVIC_IRQChannelSubPriority = request._sub,
    //     .NVIC_IRQChannelCmd = en
    // };

    // NVIC_Init(&NVIC_InitStructure);


    NVIC_SetPriority(_irq, (request._pre << 7) | (request._sub << 4));

    if(en){
        NVIC_EnableIRQ(_irq);
    }else{
        NVIC_DisableIRQ(_irq);
    }
}

void NvicPriority::enable(const IRQn _irq, const bool en) const {
    enable(*this, _irq, en);
}

void NvicRequest::enable(const bool en) const {
    NvicPriority::enable(this->_priority, this->_irq, en);
}

void NvicRequest::enable(const NvicRequest & request, const bool en){
    NvicPriority::enable(request._priority, request._irq, en);
}