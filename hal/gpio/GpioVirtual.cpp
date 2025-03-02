#include "gpiovirtual.hpp"
#include "gpio.hpp"
#include "port.hpp"

using namespace ymd::hal;
VGpio::VGpio(const Gpio & gpio):GpioIntf(gpio.pin_index), _port(form_gpiotypedef_to_port(uint32_t(gpio.instance))){;}
VGpio::VGpio(GpioPortIntf & port, const int8_t _pin_index):GpioIntf(_pin_index), _port(port){;}
VGpio::VGpio(GpioPortIntf & port, const Pin _pin):GpioIntf(CTZ((uint16_t)_pin)), _port(port){;}
GpioPortIntf & VGpio::form_gpiotypedef_to_port(uint32_t base){
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