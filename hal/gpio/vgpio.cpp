#include "vgpio.hpp"
#include "gpio.hpp"
#include "gpio_port.hpp"

using namespace ymd::hal;
VGpio::VGpio(const Gpio & gpio):
    port_(form_gpiotypedef_to_port(uint32_t(gpio.inst()))), pin_index_(gpio.index()){;}
VGpio::VGpio(GpioPortIntf & port, const int8_t pin_index):
    port_(port), pin_index_(pin_index){;}
VGpio::VGpio(GpioPortIntf & port, const Pin pin):
    port_(port), pin_index_(CTZ(uint16_t(pin))){;}


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