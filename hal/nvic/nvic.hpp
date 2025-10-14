#pragma once

#include "core/platform.hpp"
#include "core/sdk.hpp"

#if defined(WCH) && defined(RISCV)
#define SUPPORT_VTF
#endif

namespace ymd::hal{

namespace nvic_details{
void enable_nvic(const uint8_t pre, const uint8_t sub, const IRQn _irq, const Enable en);
}

struct NvicPriority;
struct NvicRequest;

struct [[nodiscard]] NvicRequest{
public:
    constexpr NvicRequest(const uint8_t pre, const uint8_t sub, const IRQn irqn):
            pre_(pre), sub_(sub), irqn_(irqn){;}

    void enable(const Enable en) const{
        nvic_details::enable_nvic(pre_, sub_, irqn_, en);
    }

private:    
    const uint8_t pre_;
    const uint8_t sub_;
    const IRQn irqn_;
};

struct [[nodiscard]] NvicPriority{
public:
    constexpr NvicPriority(
        const uint8_t pre, 
        const uint8_t sub
    ):
        pre_(pre & 0b1), 
        sub_(sub & 0b111){;}

    [[nodiscard]] constexpr NvicRequest with_irqn(const IRQn irqn) const {
        return NvicRequest(pre_, sub_, irqn);
    }
private:
    const uint8_t pre_;
    const uint8_t sub_;

    friend struct NvicRequest;
};


#ifdef SUPPORT_VTF
struct VtfRequest{
    constexpr VtfRequest(const IRQn & _irq, const uint8_t _index, void * _func):
        irqn(_irq), 
        index(_index), 
        func_base(uint32_t(_func)){;}
    void enable(const Enable en){
        SetVTFIRQ(func_base, irqn, index, en == EN);
    }

private:
    const IRQn irqn;
    const uint8_t index;
    const uint32_t func_base;
};
#endif

using PficRequest = NvicRequest;

}