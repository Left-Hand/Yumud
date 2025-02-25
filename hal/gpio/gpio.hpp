#pragma once

#include "GpioConcept.hpp"
#include "gpio_utils.hpp"

#include "sys/core/sdk.h"

namespace ymd{



class Exti;

class Gpio:public GpioConcept{
protected:
    volatile GPIO_TypeDef * instance = nullptr;
    const uint16_t pin;
    const uint32_t pin_mask;

    volatile uint32_t * pin_cfg;

    Gpio(GPIO_TypeDef * _instance,const Pin _pin):
        GpioConcept((_pin != Pin::None) ? int(CTZ((uint16_t)_pin)) : -1),
        instance(_instance),

        #if defined(CH32V20X) || defined(CH32V30X)
        pin(((_instance == GPIOC) && 
            (((*(uint32_t *) 0x40022030) & 0x0F000000) == 0)//MCU version for wch mcu, see wch sdk
            ) ? (((uint16_t)_pin >> 13)) : (uint16_t)_pin),
        #elif defined(USE_STM32_HAL_LIB)
        pin((uint16_t)_pin),
        #endif

        pin_mask(~(0xf << ((CTZ(pin) % 8) * 4))),
        pin_cfg(CTZ(pin) >= 8 ? &((instance -> CFGHR)) : &((instance -> CFGLR))){
    }

    friend class GpioVirtual;
    friend class ExtiChannel;
    friend class Port;


public:
    using GpioConcept::operator=;

    Gpio(const Gpio & other) = delete;
    Gpio(Gpio && other) = delete;
    ~Gpio(){};


    static Gpio & null();

    __fast_inline void set()override{
        instance->BSHR = pin;
    }
    __fast_inline void clr()override{
        instance->BCR = pin;
    }
    __fast_inline void write(const bool val)override{(val) ? instance->BSHR = pin : instance->BCR = pin;}
    __fast_inline bool read() const override{return (bool)(instance->INDR & pin);}

    void setMode(const GpioMode mode) override;
    __fast_inline volatile GPIO_TypeDef * inst() {return instance;} 
};


// extern Gpio GpioNull;
static inline Gpio & GpioNull = Gpio::null();
}