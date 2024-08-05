#include "port.hpp"
#include "gpio.hpp"

Gpio Port::channel_none = Gpio(GPIOD, Pin::None);

void Port::setMode(const int index, const PinMode mode){
    Gpio gpio = Gpio(instance, (Pin)(1 << index));
    gpio.setMode(mode);
}


void Port::enableRcc(const bool en){
    switch((uint32_t)instance){
        #ifdef HAVE_GPIOA
        case GPIOA_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, en);
            break;
        #endif
        #ifdef HAVE_GPIOB
        case GPIOB_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, en);
            break;
        #endif
        #ifdef HAVE_GPIOC
        case GPIOC_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, en);
            break;
        #endif
        #ifdef HAVE_GPIOD
        case GPIOD_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, en);
            break;
        #endif
        #ifdef HAVE_GPIOE
        case GPIOE_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, en);
            break;
        #endif
        default:
            break;
    }
}

void Port::init(){
    enableRcc();
}

#ifdef HAVE_GPIOA
Port portA = Port(GPIOA);
#endif

#ifdef HAVE_GPIOB
Port portB = Port(GPIOB);
#endif

#ifdef HAVE_GPIOC
Port portC = Port(GPIOC);
#endif

#ifdef HAVE_GPIOD
Port portD = Port(GPIOD);
#endif

#ifdef HAVE_GPIOE
Port portE = Port(GPIOE);
#endif

