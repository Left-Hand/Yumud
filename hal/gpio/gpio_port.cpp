#include "gpio_port.hpp"
#include "gpio.hpp"

using namespace ymd::hal;

void GpioPort::set_mode(const size_t index, const GpioMode mode){
    Gpio gpio = Gpio(
        inst_, 
        std::bit_cast<PinNth>(uint16_t(1 << index))
    );
    gpio.set_mode(mode);
}

void GpioPort::enable_rcc(const Enable en){
    switch(reinterpret_cast<uint32_t>(inst_)){
        #ifdef ENABLE_GPIOA
        case GPIOA_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, en == EN);
            break;
        #endif
        #ifdef ENABLE_GPIOB
        case GPIOB_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, en == EN);
            break;
        #endif
        #ifdef ENABLE_GPIOC
        case GPIOC_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, en == EN);
            break;
        #endif
        #ifdef ENABLE_GPIOD
        case GPIOD_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, en == EN);
            break;
        #endif
        #ifdef ENABLE_GPIOE
        case GPIOE_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, en == EN);
            break;
        #endif
        default:
            break;
    }
}

void GpioPort::init(){
    enable_rcc();
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