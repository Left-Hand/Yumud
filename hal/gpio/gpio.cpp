#include "gpio.hpp"
#include "port.hpp"

//platform independent

PortConcept & GpioVirtual::form_gpiotypedef_to_port(volatile GPIO_TypeDef * _instance){
    switch((uint32_t)_instance){
        default:
        #ifdef HAVE_GPIOA
        case GPIOA_BASE:
            return portA;
        #endif
        #ifdef HAVE_GPIOB
        case GPIOB_BASE:
            return portB;
        #endif
        #ifdef HAVE_GPIOC
        case GPIOC_BASE:
            return portC;
        #endif
        #ifdef HAVE_GPIOD
        case GPIOD_BASE:
            return portD;
        #endif
        #ifdef HAVE_GPIOE
        case GPIOE_BASE:
            return portE;
        #endif
    }
}

void Gpio::setMode(const PinMode mode){
    if(!isValid()) return;
    uint32_t tempreg = *pin_cfg;
    tempreg &= pin_mask;
    tempreg |= ((uint8_t)mode << ((pin_index % 8) * 4));
    *pin_cfg = tempreg;

    if(mode == PinMode::InPullUP){
        instance -> OUTDR |= pin;
    }else if(mode == PinMode::InPullDN){
        instance -> OUTDR &= ~pin;
    }
}