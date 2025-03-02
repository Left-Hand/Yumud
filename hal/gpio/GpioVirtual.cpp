#include "gpiovirtual.hpp"
#include "gpio.hpp"
#include "port.hpp"

using namespace ymd::hal;
VGpio::VGpio(const Gpio & gpio):GpioConcept(gpio.pin_index), _port(form_gpiotypedef_to_port(uint32_t(gpio.instance))){;}
VGpio::VGpio(GpioPortConcept & port, const int8_t _pin_index):GpioConcept(_pin_index), _port(port){;}
VGpio::VGpio(GpioPortConcept & port, const Pin _pin):GpioConcept(CTZ((uint16_t)_pin)), _port(port){;}
GpioPortConcept & VGpio::form_gpiotypedef_to_port(uint32_t base){
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