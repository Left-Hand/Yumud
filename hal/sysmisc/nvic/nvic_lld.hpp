#pragma once


#include "nvic_primitive.hpp"
#include "core/irqn.hpp"

namespace ymd::lld { 

void nvic_enable_irqn(
    const IRQn irqn,
    const bool en
);

void nvic_set_irqn_priority(
    const IRQn irqn,
    const hal::NvicPriorityCode priority
);


}