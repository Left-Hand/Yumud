#pragma once

#include "timer_primitive.hpp"
#include "hal/gpio/gpio.hpp"
#include "timer_layout.hpp"
#include "core/irqn.hpp"

namespace ymd::hal::timer{

namespace details{
[[nodiscard]] bool is_basic_timer(const void * inst);

[[nodiscard]] bool is_generic_timer(const void * inst);

[[nodiscard]] bool is_advanced_timer(const void * inst);

[[nodiscard]] IRQn it_to_irq(const void * inst, const TimerIT it);


std::tuple<uint16_t, uint16_t> calc_arr_and_psc(
    const uint32_t aligned_bus_clk_freq,
    const TimerCountFreq count_freq
);

void set_remap(void* inst_, const TimerRemap rm);
void enable_rcc(void* inst_, const Enable en);
}

};
