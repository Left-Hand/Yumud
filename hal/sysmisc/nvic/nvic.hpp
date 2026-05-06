#pragma once

#include "core/platform.hpp"
#include "core/irqn.hpp"
#include "nvic_primitive.hpp"
#include "nvic_lld.hpp"

#if defined(WCH) && defined(RISCV)
#define SUPPORT_VTF
#endif

namespace ymd::hal{


// struct NvicPriority;
// struct NvicRequest;

// struct [[nodiscard]] NvicRequest{
// public:
//     hal::NvicPriorityCode priority_code;
//     const IRQn irqn;

// };

// struct [[nodiscard]] NvicPriority{
// public:

//     hal::NvicPriorityCode priority_code;

//     [[nodiscard]] constexpr NvicRequest with_irqn(const IRQn irqn) const noexcept {
//         return NvicRequest(priority_code, irqn);
//     }
// private:
//     friend struct NvicRequest;
// };


#ifdef SUPPORT_VTF
struct VtfRequest{
    constexpr VtfRequest(const IRQn & _irq, const uint8_t _index, void * _func):
        irqn(_irq), 
        index(_index), 
        func_base(uint32_t(_func)){;}
    void enable(const Enable en){
        SetVTFIRQ(func_base, irqn, index, (en == EN));
    }

private:
    const IRQn irqn;
    const uint8_t index;
    const uint32_t func_base;
};
using PficRequest = NvicRequest;
#endif


}