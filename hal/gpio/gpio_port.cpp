#include "gpio_port.hpp"
#include "gpio.hpp"
#include "core/sdk.hpp"

using namespace ymd;
using namespace ymd::hal;


#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, GPIO_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


void GpioPort::set_mode(const Nth nth, const GpioMode mode){
    Gpio gpio = Gpio(
        inst_, 
        std::bit_cast<PinSource>(uint16_t(1 << nth.count()))
    );
    gpio.set_mode(mode);
}

void gpio_enable_rcc(const void * inst, const Enable en){
    switch(reinterpret_cast<size_t>(inst)){
        #ifdef GPIOA_PRESENT
        case GPIOA_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, en == EN);
            break;
        #endif
        #ifdef GPIOB_PRESENT
        case GPIOB_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, en == EN);
            break;
        #endif
        #ifdef GPIOC_PRESENT
        case GPIOC_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, en == EN);
            break;
        #endif
        #ifdef GPIOD_PRESENT
        case GPIOD_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, en == EN);
            break;
        #endif
        #ifdef GPIOE_PRESENT
        case GPIOE_BASE:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, en == EN);
            break;
        #endif
    }

    __builtin_trap();
}

void GpioPort::enable_rcc(const Enable en){
    gpio_enable_rcc(inst_, en);
}

void GpioPort::init(){
    enable_rcc(EN);
}


void GpioPort::set_by_mask(const PinMask mask){
    SDK_INST(inst_)->BSHR = mask.to_u16();
}

void GpioPort::clr_by_mask(const PinMask mask){
    SDK_INST(inst_)->BCR = mask.to_u16();
}

void GpioPort::write_by_mask(const PinMask mask){
    SDK_INST(inst_)->OUTDR = mask.to_u16();}
PinMask GpioPort::read_mask(){
    return PinMask::from_u16(SDK_INST(inst_)->INDR);
}

void GpioPort::write_nth(const Nth nth, const BoolLevel data){
    const auto mask = PinMask::from_nth(nth);
    if(data == HIGH){
        set_by_mask(mask);
    }else{
        clr_by_mask(mask);
    }
}