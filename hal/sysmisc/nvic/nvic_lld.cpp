#include "nvic_lld.hpp"
#include "core/sdk.hpp"
#include <type_traits>

namespace ymd::lld{

using SplInst = std::remove_pointer_t<decltype(NVIC)>;
static constexpr size_t DE_REALTIVE_OFFSET_TO_EN_32 = 
    (__builtin_offsetof(SplInst, SplInst::IRER[0]) 
        - __builtin_offsetof(SplInst, SplInst::IENR[0])) / sizeof(SplInst::IRER[0]);

void nvic_enable_irqn(
    const IRQn irqn,
    const bool en
){
    auto * p = &NVIC->IENR[0];

    int32_t offset = 0;
    offset += (en == false)? DE_REALTIVE_OFFSET_TO_EN_32 : 0;
    offset += (uint32_t)(irqn) >> 5;
    p[offset] = (1 << ((uint32_t)(irqn) & 0x1F));
}

void nvic_set_irqn_priority(
    const IRQn irqn,
    const hal::NvicPriorityCode priority
){
    NVIC->IPRIOR[(uint32_t)(irqn)] = priority.bits;
}

}