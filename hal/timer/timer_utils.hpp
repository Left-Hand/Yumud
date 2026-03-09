#pragma once

#include "timer_primitive.hpp"
#include "hal/gpio/gpio.hpp"
#include "timer_layout.hpp"
#include "core/irqn.hpp"
#include "core/utils/nth.hpp"

namespace ymd::hal::timer{

namespace details{

Nth timer_to_nth(const uintptr_t inst_base);

[[nodiscard]] static constexpr bool is_basic_timer(const Nth nth){
    switch(nth.count()){
        #ifdef TIM6_PRESENT
        case 6: return true;
        #endif

        #ifdef TIM7_PRESENT
        case 7: return true;
        #endif

    }
    return false;
}

[[nodiscard]] static constexpr bool is_generic_timer(const Nth nth){
    switch(nth.count()){
        #ifdef TIM2_PRESENT
        case 2: return true;
        #endif

        #ifdef TIM3_PRESENT
        case 3: return true;
        #endif

        #ifdef TIM4_PRESENT
        case 4: return true;
        #endif

        #ifdef TIM5_PRESENT
        case 5: return true;
        #endif
    }
    return false;
}

[[nodiscard]] static constexpr bool is_advanced_timer(const Nth nth){
    switch(nth.count()){
        #ifdef TIM1_PRESENT
        case 1: return true;
        #endif

        #ifdef TIM8_PRESENT
        case 8: return true;
        #endif

        #ifdef TIM9_PRESENT
        case 9: return true;
        #endif

        #ifdef TIM10_PRESENT
        case 0: return true;
        #endif
    
    }
    return false;
}

[[nodiscard]] static constexpr IRQn it_to_irq(const Nth nth, const TimerIT it){

    #define GENERIC_TIMER_IRQ_TEMPLATE(x)\
    case x:\
        return TIM##x##_IRQn;\

    #define ADVANCED_TIMER_IRQ_TEMPLATE(x)\
    case x:\
        switch(it){\
            case TimerIT::Update:\
                return TIM##x##_UP_IRQn;\
            case TimerIT::CC1:\
            case TimerIT::CC2:\
            case TimerIT::CC3:\
            case TimerIT::CC4:\
                return TIM##x##_CC_IRQn;\
            case TimerIT::Trigger:\
            case TimerIT::COM:\
                return TIM##x##_TRG_COM_IRQn;\
            case TimerIT::Break:\
                return TIM##x##_BRK_IRQn;\
        }\
        break;\

    switch(nth.count()){
        #ifdef TIM1_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(1)
        #endif

        #ifdef TIM2_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(2)
        #endif

        #ifdef TIM3_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(3)
        #endif

        #ifdef TIM4_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(4)
        #endif

        #ifdef TIM5_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(5)
        #endif

        #ifdef TIM6_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(6)
        #endif

        #ifdef TIM7_PRESENT
        GENERIC_TIMER_IRQ_TEMPLATE(7)
        #endif

        #ifdef TIM8_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(8)
        #endif

        #ifdef TIM9_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(9)
        #endif

        #ifdef TIM10_PRESENT
        ADVANCED_TIMER_IRQ_TEMPLATE(10)
        #endif

    }
    __builtin_trap();

    #undef GENERIC_TIMER_IRQ_TEMPLATE
    #undef ADVANCED_TIMER_IRQ_TEMPLATE
}



std::tuple<uint16_t, uint16_t> calc_arr_and_psc(
    const uint32_t aligned_bus_clk_freq,
    const TimerCountFreq count_freq
);

void set_remap(const Nth nth, const TimerRemap rm);
void enable_rcc(const Nth nth, const Enable en);
}

};
