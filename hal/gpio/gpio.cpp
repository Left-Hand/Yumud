#include "gpio.hpp"
#include "port.hpp"

using namespace ymd;

PortConcept & GpioVirtual::form_gpiotypedef_to_port(uint32_t base){
    switch(base){
        default:
        #ifdef ENABLE_GPIOA
        case GPIOA_BASE:
            return portA;
        #endif
        #ifdef ENABLE_GPIOB
        case GPIOB_BASE:
            return portB;
        #endif
        #ifdef ENABLE_GPIOC
        case GPIOC_BASE:
            return portC;
        #endif
        #ifdef ENABLE_GPIOD
        case GPIOD_BASE:
            return portD;
        #endif
        #ifdef ENABLE_GPIOE
        case GPIOE_BASE:
            return portE;
        #endif
    }
}

void Gpio::setMode(const GpioMode mode){
    if(!isValid()) return;
    uint32_t tempreg = *pin_cfg;
    tempreg &= pin_mask;
    tempreg |= ((uint8_t)mode << ((pin_index % 8) * 4));
    *pin_cfg = tempreg;

    if(mode == GpioMode::InPullUP){
        instance -> OUTDR |= pin;
    }else if(mode == GpioMode::InPullDN){
        instance -> OUTDR &= ~pin;
    }
}