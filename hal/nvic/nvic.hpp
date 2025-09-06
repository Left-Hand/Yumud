#pragma once

#include "core/platform.hpp"
#include "core/sdk.hpp"

#if defined(WCH) && defined(RISCV)
#define SUPPORT_VTF
#endif

namespace ymd::hal{

struct NvicPriority;
class NvicRequest;

struct NvicPriority{
protected:
    const uint8_t _pre;
    const uint8_t _sub;
public:
    NvicPriority(const uint8_t pre, const uint8_t sub);

    static void enable(const NvicPriority & request, const IRQn _irq, const Enable en);

    void enable(const IRQn _irq, const Enable en) const;


};


class NvicRequest{
protected:
    const NvicPriority _priority;
    const IRQn _irq;
public:
    NvicRequest(const uint8_t pre, const uint8_t sub, const IRQn irq):
            _priority(pre, sub), _irq(irq){;}

    NvicRequest(const NvicPriority priority, const IRQn irq):
            _priority(priority), _irq(irq){;}

    void enable(const Enable en) const ;

    static void enable(const NvicRequest & request, const Enable en);
};

#ifdef SUPPORT_VTF
class VtfRequest{
protected:
    const IRQn irq;
    const uint8_t index;
    const uint32_t func_base;
public:
    VtfRequest(const IRQn & _irq, const uint8_t _index, void * _func):irq(_irq), index(_index), func_base(uint32_t(_func)){;}
    void enable(const Enable en){
        SetVTFIRQ(func_base, irq, index, en == EN);
    }
};
#endif

using PficRequest = NvicRequest;

}