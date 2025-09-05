#include "vgpio.hpp"
#include "gpio.hpp"
#include "gpio_port.hpp"

using namespace ymd::hal;
VGpio::VGpio(const Gpio & gpio):
    port_(form_gpiotypedef_to_port(uint32_t(gpio.inst()))), nth_(gpio.nth()){;}
VGpio::VGpio(GpioPortIntf & port, const PinNth pin_nth):
    port_(port), nth_(Nth(CTZ(uint16_t(pin_nth)))){;}


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