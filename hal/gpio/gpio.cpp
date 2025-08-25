#include "gpio_port_intf.hpp"
#include "gpio.hpp"

#include "core/sdk.hpp"

using namespace ymd::hal;

void Gpio::set_mode(const GpioMode mode){
    if(not is_valid()) return;
    const auto ctz_pin = CTZ(uint16_t(pin_));
    auto & pin_cfg = (ctz_pin >= 8 ? ((instance_ -> CFGHR)) : ((instance_ -> CFGLR)));
    uint32_t tempreg = pin_cfg;
    const auto shifts = ((ctz_pin % 8) * 4);
    tempreg &= (~(0xf << shifts));
    tempreg |= (mode.as_u8() << shifts);
    pin_cfg = tempreg;

    if(mode == GpioMode::InPullUP){
        instance_ -> OUTDR |= uint16_t(pin_);
    }else if(mode == GpioMode::InPullDN){
        instance_ -> OUTDR &= ~uint16_t(pin_);
    }
}
