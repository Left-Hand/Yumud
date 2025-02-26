#include "port_concept.hpp"
#include "Gpio.hpp"
#include "sys/core/sdk.h"

using namespace ymd;



void Gpio::setMode(const GpioMode mode){
    if(!isValid()) return;
    uint32_t tempreg = *pin_cfg;
    tempreg &= pin_mask;
    tempreg |= ((uint8_t)mode << ((pin_index % 8) * 4));
    *pin_cfg = tempreg;

    if(mode == GpioMode::InPullUP){
        instance -> OUTDR |= pin;
    }else if(mode == GpioMode::InPullDN){
        instance -> OUTDR &= ~pin;
    }
}

Gpio & Gpio::null(){
    static Gpio GpioNull = Gpio(GPIOD, Pin::None);
    return GpioNull;
}

// namespace ymd{
// Gpio & GpioNull = Gpio::null();
// }