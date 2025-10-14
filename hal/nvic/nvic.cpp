#include "nvic.hpp"

void ymd::hal::nvic_details::
enable_nvic(
    const uint8_t pre, 
    const uint8_t sub, 
    const IRQn _irq, 
    const Enable en
){
    NVIC_SetPriority(_irq, (pre << 7) | (sub << 4));

    if(en == EN){
        NVIC_EnableIRQ(_irq);
    }else{
        NVIC_DisableIRQ(_irq);
    }
}

