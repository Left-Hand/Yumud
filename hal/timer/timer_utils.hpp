#pragma once

#include "timer_primitive.hpp"
#include "core/sdk.hpp"

namespace ymd::hal{
class Gpio;


namespace details{
[[nodiscard]] bool is_basic_timer(const void * inst);

[[nodiscard]] bool is_generic_timer(const void * inst);

[[nodiscard]] bool is_advanced_timer(const void * inst);

IRQn it_to_irq(const void * inst, const TimerIT it);

Gpio get_pin(const void * inst, const TimerChannelSelection channel);

constexpr bool is_aligned_count_mode(const TimerCountMode mode){
    return  (mode == TimerCountMode::CenterAlignedDownTrig) ||
            (mode == TimerCountMode::CenterAlignedUpTrig) ||
            (mode == TimerCountMode::CenterAlignedDualTrig);
}
}

};
