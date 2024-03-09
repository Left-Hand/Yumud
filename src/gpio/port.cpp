#include "port.hpp"

void PortBase::setByIndex(const int8_t index){
    setBits(1 << index);
}

void PortBase::clrByIndex(const int8_t index){
    clrBits(1 << index);
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

void Port::setBits(const uint16_t & data){
    instance->BSHR = data;
}

void Port::set(const Pin & pin){
    instance->BSHR = (uint16_t)pin;
}

void Port::clrBits(const uint16_t & data){
    instance->BCR = data;
}

void Port::clr(const Pin & pin){
    instance->BCR = (uint16_t)pin;
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

