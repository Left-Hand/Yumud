#include "gpio_port_intf.hpp"
#include "gpio.hpp"

#include "core/sdk.hpp"

using namespace ymd::hal;


Gpio::Gpio(GPIO_TypeDef * inst, const PinNth pin):
    inst_(inst)

    #if defined(CH32V20X) || defined(CH32V30X)
    ,pin_nth_(PinNth(
        (inst == GPIOC) && 
        (
            ((* reinterpret_cast<uint32_t *> (0x40022030) & 0x0F000000) == 0)//MCU version for wch mcu, see wch sdk
        ) ? uint16_t(uint16_t(pin) >> 13) : uint16_t(pin)
    ))
    #elif defined(USE_STM32_HAL_LIB)
    ,pin_nth_(pin)
    #endif
    {}


void Gpio::set_mode(const GpioMode mode){
    const auto ctz_pin = CTZ(uint16_t(pin_nth_));
    auto & pin_cfg = (ctz_pin >= 8 ? ((inst_ -> CFGHR)) : ((inst_ -> CFGLR)));
    uint32_t tempreg = pin_cfg;
    const auto shifts = ((ctz_pin % 8) * 4);
    tempreg &= (~(0xf << shifts));
    tempreg |= (mode.as_u8() << shifts);
    pin_cfg = tempreg;

    if(mode == GpioMode::InPullUP){
        inst_ -> OUTDR |= uint16_t(pin_nth_);
    }else if(mode == GpioMode::InPullDN){
        inst_ -> OUTDR &= ~uint16_t(pin_nth_);
    }
}

PortSource Gpio::port() const {
    const auto base = reinterpret_cast<uint32_t>(inst_);
    switch(base){
        default:
        #ifdef ENABLE_GPIOA
        case GPIOA_BASE:
            return PortSource::PA;
        #endif
        #ifdef ENABLE_GPIOB
        case GPIOB_BASE:
            return PortSource::PB;
        #endif
        #ifdef ENABLE_GPIOC
        case GPIOC_BASE:
            return PortSource::PC;
        #endif
        #ifdef ENABLE_GPIOD
        case GPIOD_BASE:
            return PortSource::PD;
        #endif
        #ifdef ENABLE_GPIOE
        case GPIOE_BASE:
            return PortSource::PE;
        #endif
        #ifdef ENABLE_GPIOF
        case GPIOF_BASE:
            return PortSource::PF;
        #endif
    }
}