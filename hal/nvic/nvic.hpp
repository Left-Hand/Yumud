#ifndef __NVIC_HPP__

#define __NVIC_HPP__

#include "../sys/core/platform.h"

#if defined(WCH) && defined(RISCV)
#define SUPPORT_VTF
#endif

struct NvicPriority;
class NvicRequest;

struct NvicPriority{
protected:
    const uint8_t pre;
    const uint8_t sub;
public:
    NvicPriority() = delete;
    NvicPriority(const uint8_t _pre, const uint8_t _sub):pre(CLAMP(_pre,0,1)), sub(CLAMP(_sub, 0,7)){;}

    static void enable(const NvicPriority request, const IRQn _irq, const bool en = true){
        NVIC_InitTypeDef NVIC_InitStructure = {0};
        NVIC_InitStructure.NVIC_IRQChannel = _irq;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = request.pre;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = request.sub;
        NVIC_InitStructure.NVIC_IRQChannelCmd = en;
        NVIC_Init(&NVIC_InitStructure);
    }

    void enable(const IRQn _irq, const bool en = true) const {
        enable(*this, _irq, en);
    }
};


class NvicRequest:public NvicPriority{
protected:
    const IRQn irq;
public:
    NvicRequest(const uint8_t _pre, const uint8_t _sub, const IRQn _irq):
            NvicPriority(_pre, _sub), irq(_irq){;}

    NvicRequest(const NvicPriority priority, const IRQn _irq):
            NvicPriority(priority), irq(_irq){;}

    void enable(const bool en = true) const {
        NvicPriority::enable(*this, this->irq, en);
    }

    static void enable(const NvicRequest request, const bool en = true){
        NvicPriority::enable(request, request.irq, en);
    }

};

#ifdef SUPPORT_VTF
class VtfRequest{
protected:
    const IRQn irq;
    const uint8_t index;
    const uint32_t func_base;
public:
    VtfRequest(const IRQn & _irq, const uint8_t & _index, void * _func):irq(_irq), index(_index), func_base((uint32_t)_func){;}
    void enable(const bool & en = true){
        SetVTFIRQ(func_base, irq, index, en);
    }
};
#endif

typedef NvicRequest PficRequest;

#endif