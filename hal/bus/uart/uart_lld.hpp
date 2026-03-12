#pragma once

#include <functional>


#include "hal/bus/bus_enums.hpp"
#include "primitive/hal_result.hpp"
#include "uart_primitive.hpp"
#include "uart_layout.hpp"


#include "core/utils/nth.hpp"

namespace ymd::lld{

void usart_enable_error_interrupt(void * p_inst_, const Enable en);

void uart_enable_rxne_interrupt(void * p_inst, const Enable en);

void uart_enable_tx_interrupt(void * p_inst, const Enable en);

void uart_enable_idle_interrupt(void * p_inst, const Enable en);

void uart_enable_rcc(const Nth nth, const Enable en);

void uart_set_remap(const Nth nth, const hal::UartRemap remap);

Nth uart_to_nth(const uintptr_t inst_base);
static constexpr IRQn uart_calc_nvic_irqn(const Nth nth){

    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            return USART1_IRQn;
        #endif
        #ifdef USART2_PRESENT
        case 2:
            return USART2_IRQn;
        #endif
        #ifdef USART3_PRESENT
        case 3:
            return USART3_IRQn;
        #endif
        #ifdef UART4_PRESENT
        case 4:
            return UART4_IRQn;
        #endif
        #ifdef UART5_PRESENT
        case 5:
            return UART5_IRQn;
        #endif
        #ifdef UART6_PRESENT
        case 6:
            return UART6_IRQn;
        #endif
        #ifdef UART7_PRESENT
        case 7:
            return UART7_IRQn;
        #endif
        #ifdef UART8_PRESENT
        case 8:
            return UART8_IRQn;
        #endif
    }
    __builtin_trap();
}
}