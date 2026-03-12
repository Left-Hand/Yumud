#include "uart_lld.hpp"
#include "core/sdk.hpp"


using namespace ymd;
using namespace ymd::lld;
using namespace ymd::hal;



#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SPL_INST(x) (reinterpret_cast<COPY_CONST(x, USART_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))



namespace ymd::lld{ 

void usart_enable_error_interrupt(void * p_inst_, const Enable en){
	USART_ITConfig(SPL_INST(p_inst_), USART_IT_PE, (en == EN));
	USART_ITConfig(SPL_INST(p_inst_), USART_IT_ERR, (en == EN));
}

void uart_enable_rxne_interrupt(void * p_inst, const Enable en){
    USART_ClearITPendingBit(SPL_INST(p_inst), USART_IT_RXNE);
    USART_ITConfig(SPL_INST(p_inst), USART_IT_RXNE, (en == EN));
}


void uart_enable_tx_interrupt(void * p_inst, const Enable en){
    USART_ITConfig(SPL_INST(p_inst), USART_IT_TXE, (en == EN));
}

void uart_enable_idle_interrupt(void * p_inst, const Enable en){ 
    USART_ITConfig(SPL_INST(p_inst), USART_IT_IDLE, (en == EN));
}


Nth uart_to_nth(const uintptr_t inst_base){
    switch(inst_base){
        #ifdef USART1_PRESENT
        case USART1_BASE:
            return Nth(1);
        #endif
        #ifdef USART2_PRESENT
        case USART2_BASE:
            return Nth(2);
        #endif
        #ifdef USART3_PRESENT
        case USART3_BASE:
            return Nth(3);
        #endif
        #ifdef UART4_PRESENT
        case UART4_BASE:
            return Nth(4);
        #endif
        #ifdef UART5_PRESENT
        case UART5_BASE:
            return Nth(5);
        #endif
        #ifdef UART6_PRESENT
        case UART6_BASE:
            return Nth(6);
        #endif
        #ifdef UART7_PRESENT
        case UART7_BASE:
            return Nth(7);
        #endif
        #ifdef UART8_PRESENT
        case UART8_BASE:
            return Nth(8);
        #endif
    }
    __builtin_trap();
}

void uart_enable_rcc(const Nth nth, const Enable en){
    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, (en == EN));
            return;
        #endif
        #ifdef USART2_PRESENT
        case 2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, (en == EN));
            return;
        #endif
        #ifdef USART3_PRESENT
        case 3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, (en == EN));
            return;
        #endif
        #ifdef UART4_PRESENT
        case 4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, (en == EN));
            return;
        #endif
        #ifdef UART5_PRESENT
        case 5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, (en == EN));
            return;
        #endif
        #ifdef UART6_PRESENT
        case 6:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, (en == EN));
            return;
        #endif
        #ifdef UART7_PRESENT
        case 7:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, (en == EN));
            return;
        #endif
        #ifdef UART8_PRESENT
        case 8:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, (en == EN));
            return;
        #endif
    }
    __builtin_trap();
}

void uart_set_remap(const Nth nth, const UartRemap remap){
    switch(nth.count()){
        #ifdef USART1_PRESENT
        case 1:
            switch(remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap_USART1, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
                    return;
                default:
                    break;
            }
        #endif
        #ifdef USART2_PRESENT
        case 2:
            switch(remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_Remap_USART2, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
                    return;
                default:
                    break;
            }
        #endif
        #ifdef USART3_PRESENT
        case 3:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap1_USART3, ENABLE);
                    return;
                case UartRemap::_2:
                    GPIO_PinRemapConfig(GPIO_PartialRemap2_USART3, ENABLE);
                    return;
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART4_PRESENT
        case 4:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART4, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART4, ENABLE);
                    return;
            }
            break;
        #endif
        #ifdef UART5_PRESENT
        case 5:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART5, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART5, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART6_PRESENT
        case 6:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART6, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART6, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART7_PRESENT
        case 7:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART7, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART7, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
        #ifdef UART8_PRESENT
        case 8:
            switch (remap){
                case UartRemap::_0:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART8, DISABLE);
                    return;
                case UartRemap::_1:
                    GPIO_PinRemapConfig(GPIO_PartialRemap_USART8, ENABLE);
                    return;
                case UartRemap::_2:
                case UartRemap::_3:
                    GPIO_PinRemapConfig(GPIO_FullRemap_USART8, ENABLE);
                    return;
                default:
                    break;
            }
            break;
        #endif
    }
    __builtin_trap();
}

}