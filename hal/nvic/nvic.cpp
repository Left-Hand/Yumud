#include "nvic.hpp"

#include "core/sdk.hpp"

void ymd::hal::nvic_details::
enable_nvic(
    const uint8_t pre, 
    const uint8_t sub, 
    const IRQn irqn, 
    const Enable en
){
    NVIC_SetPriority(irqn, (pre << 7) | (sub << 4));

    if(en == EN){
        NVIC_EnableIRQ(irqn);
    }else{
        NVIC_DisableIRQ(irqn);
    }
}

