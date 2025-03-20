#include "port_intf.hpp"
#include "gpio.hpp"

#include "core/sdk.hpp"

using namespace ymd::hal;

void Gpio::setMode(const GpioMode mode){
    if(!valid()) return;
    const auto ctz_pin = CTZ(uint16_t(pin_));
    auto & pin_cfg = (ctz_pin >= 8 ? ((instance_ -> CFGHR)) : ((instance_ -> CFGLR)));
    uint32_t tempreg = pin_cfg;
    const auto shifts = ((ctz_pin % 8) * 4);
    tempreg &= (~(0xf << shifts));
    tempreg |= ((uint8_t)mode << shifts);
    pin_cfg = tempreg;

    if(mode == GpioMode::InPullUP){
        instance_ -> OUTDR |= uint16_t(pin_);
    }else if(mode == GpioMode::InPullDN){
        instance_ -> OUTDR &= ~uint16_t(pin_);
    }
}

Gpio & Gpio::null(){
    static Gpio NullGpio = Gpio(GPIOD, Pin::None);
    return NullGpio;
}

namespace ymd::hal{
Gpio & NullGpio = Gpio::null();
}