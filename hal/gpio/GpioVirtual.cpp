#include "GpioVirtual.hpp"
#include "gpio.hpp"
#include "port.hpp"

using namespace ymd;
GpioVirtual::GpioVirtual(const Gpio & gpio):GpioConcept(gpio.pin_index), _port(form_gpiotypedef_to_port(uint32_t(gpio.instance))){;}
GpioVirtual::GpioVirtual(PortConcept & port, const int8_t _pin_index):GpioConcept(_pin_index), _port(port){;}
GpioVirtual::GpioVirtual(PortConcept & port, const Pin _pin):GpioConcept(CTZ((uint16_t)_pin)), _port(port){;}
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