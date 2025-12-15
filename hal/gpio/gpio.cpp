#include "gpio_port_intf.hpp"
#include "gpio.hpp"

#include "core/sdk.hpp"

namespace ymd::hal{

#define COPY_CONST(a,b) std::conditional_t<\
    std::is_const_v<std::decay_t<decltype(a)>>,\
    std::add_const_t<b *>,\
    std::remove_const_t<b *>>\

#define SDK_INST(x) (reinterpret_cast<COPY_CONST(x, GPIO_TypeDef)>(x))
#define RAL_INST(x) (reinterpret_cast<COPY_CONST(x, ral::USART_Def)>(x))


void Gpio::set_high(){
    SDK_INST(inst_)->BSHR = static_cast<uint16_t>(pin_nth_);
}
void Gpio::set_low(){
    SDK_INST(inst_)->BCR = static_cast<uint16_t>(pin_nth_);
}

//BSHR的寄存器在BCR前 {1->BSHR; 0->BCR} 使用逻辑操作而非判断以提高速度
void Gpio::write(const BoolLevel val){
    *(&SDK_INST(inst_)->BCR - int(val.to_bool())) = static_cast<uint16_t>(pin_nth_);
}

BoolLevel Gpio::read() const {
    return BoolLevel::from(SDK_INST(inst_)->INDR & static_cast<uint16_t>(pin_nth_));
}

Gpio::Gpio(void * inst, const PinSource pin):
    inst_(inst)

    #if defined(CH32V20X) || defined(CH32V30X)
    ,pin_nth_(PinSource(
        (inst == GPIOC) && 
        (
            ((* reinterpret_cast<uint32_t *> (0x40022030) & 0x0F000000) == 0)
            //MCU version for wch mcu, see wch sdk
        ) ? uint16_t(uint16_t(pin) >> 13) : uint16_t(pin)
    ))
    #elif defined(USE_STM32_HAL_LIB)
    ,pin_nth_(pin)
    #endif
    {}


void Gpio::set_mode(const GpioMode mode){
    const auto ctz_pin = CTZ(uint16_t(pin_nth_));
    auto & pin_cfg = (ctz_pin >= 8 ? ((SDK_INST(inst_) -> CFGHR)) : ((SDK_INST(inst_) -> CFGLR)));
    uint32_t tempreg = pin_cfg;
    const auto shifts = ((ctz_pin % 8) * 4);
    tempreg &= (~(0xf << shifts));
    tempreg |= (mode.to_u8() << shifts);
    pin_cfg = tempreg;

    if(mode == GpioMode::InPullUP){
        SDK_INST(inst_) -> OUTDR |= uint16_t(pin_nth_);
    }else if(mode == GpioMode::InPullDN){
        SDK_INST(inst_) -> OUTDR &= ~uint16_t(pin_nth_);
    }
}



static inline PortSource inst_to_portsource(const void * inst){
    const auto base = reinterpret_cast<uint32_t>(inst);
    switch(base){
        #ifdef GPIOA_PRESENT
        case GPIOA_BASE:
            return PortSource::PA;
        #endif
        #ifdef GPIOB_PRESENT
        case GPIOB_BASE:
            return PortSource::PB;
        #endif
        #ifdef GPIOC_PRESENT
        case GPIOC_BASE:
            return PortSource::PC;
        #endif
        #ifdef GPIOD_PRESENT
        case GPIOD_BASE:
            return PortSource::PD;
        #endif
        #ifdef GPIOE_PRESENT
        case GPIOE_BASE:
            return PortSource::PE;
        #endif
        #ifdef GPIOF_PRESENT
        case GPIOF_BASE:
            return PortSource::PF;
        #endif
    }
    __builtin_trap();
}

static inline void * portsource_to_inst(PortSource port) {
    switch (port) {
        #ifdef GPIOA_PRESENT
        case PortSource::PA: return reinterpret_cast<void *>(GPIOA_BASE);
        #endif
        #ifdef GPIOB_PRESENT
        case PortSource::PB: return reinterpret_cast<void *>(GPIOB_BASE);
        #endif
        #ifdef GPIOC_PRESENT
        case PortSource::PC: return reinterpret_cast<void *>(GPIOC_BASE);
        #endif
        #ifdef GPIOD_PRESENT
        case PortSource::PD: return reinterpret_cast<void *>(GPIOD_BASE);
        #endif
        #ifdef GPIOE_PRESENT
        case PortSource::PE: return reinterpret_cast<void *>(GPIOE_BASE);
        #endif
        #ifdef GPIOF_PRESENT
        case PortSource::PF: return reinterpret_cast<void *>(GPIOF_BASE);
        #endif
        default:
        __builtin_trap();
    }
}


PortSource Gpio::port() const {
    return inst_to_portsource(SDK_INST(inst_));
}

Gpio make_gpio(const PortSource port_source, const PinSource pin_source){
    return Gpio(portsource_to_inst(port_source), pin_source);
}

Gpio make_gpio(PortSource port_source, const Nth nth){
    return make_gpio(portsource_to_inst(port_source), nth);
}



}