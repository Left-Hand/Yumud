#include "port.hpp"
#include "gpio.hpp"

using namespace ymd;

void Port::setMode(const int index, const GpioMode mode){
    Gpio gpio = Gpio(instance, (Pin)(1 << index));
    gpio.setMode(mode);
}

// Gpio Port::operator [](const int index){
//     if(index < 0) return channel_none;

//     return Gpio(instance, Pin(1 << index));
// };
// Gpio Port::operator [](const Pin pin){
//     if(pin != Pin::None) return channel_none;

//     return Gpio(instance, pin);
// };

void Port::enableRcc(const bool en){
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

void Port::init(){
    enableRcc();
}


namespace ymd{
#ifdef ENABLE_GPIOA
Port portA{GPIOA};
#endif

#ifdef ENABLE_GPIOB
Port portB{GPIOB};
#endif

#ifdef ENABLE_GPIOC
Port portC{GPIOC};
#endif

#ifdef ENABLE_GPIOD
Port portD{GPIOD};
#endif

#ifdef ENABLE_GPIOE
Port portE{GPIOE};
#endif

}