#include "port.hpp"
#include "gpio.hpp"

using namespace ymd::hal;

void GpioPort::set_mode(const int index, const GpioMode mode){
    Gpio gpio = Gpio(instance, (Pin)(1 << index));
    gpio.set_mode(mode);
}

void GpioPort::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef ENABLE_GPIOA
        case GPIOA_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, en);
            break;
        #endif
        #ifdef ENABLE_GPIOB
        case GPIOB_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, en);
            break;
        #endif
        #ifdef ENABLE_GPIOC
        case GPIOC_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, en);
            break;
        #endif
        #ifdef ENABLE_GPIOD
        case GPIOD_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, en);
            break;
        #endif
        #ifdef ENABLE_GPIOE
        case GPIOE_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, en);
            break;
        #endif
        default:
            break;
    }
}

void GpioPort::init(){
    enableRcc();
}


namespace ymd::hal{
#ifdef ENABLE_GPIOA
GpioPort portA{GPIOA};
#endif

#ifdef ENABLE_GPIOB
GpioPort portB{GPIOB};
#endif

#ifdef ENABLE_GPIOC
GpioPort portC{GPIOC};
#endif

#ifdef ENABLE_GPIOD
GpioPort portD{GPIOD};
#endif

#ifdef ENABLE_GPIOE
GpioPort portE{GPIOE};
#endif

}