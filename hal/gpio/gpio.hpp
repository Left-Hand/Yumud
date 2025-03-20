#pragma once

#include "GpioConcept.hpp"
#include "gpio_utils.hpp"
#include "GpioTag.hpp"

#include "core/sdk.hpp"

namespace ymd::hal{

class Exti;

class Gpio final: public GpioIntf{
protected:
    GPIO_TypeDef * instance_;
    const Pin pin_;
    // const uint32_t pin_mask_;

    // volatile uint32_t & pin_cfg;

    Gpio(GPIO_TypeDef * instance, const Pin pin):
        instance_(instance)

        #if defined(CH32V20X) || defined(CH32V30X)
        ,pin_(Pin(
            (instance == GPIOC) && 
            (
                ((* reinterpret_cast<uint32_t *> (0x40022030) & 0x0F000000) == 0)//MCU version for wch mcu, see wch sdk
            ) ? uint16_t(uint16_t(pin) >> 13) : uint16_t(pin)
        ))
        #elif defined(USE_STM32_HAL_LIB)
        ,pin_(pin)
        #endif
        {
    }

    friend class VGpio;
    friend class ExtiChannel;
    friend class GpioPort;


public:
    using GpioIntf::operator=;

    Gpio(const Gpio & other) = delete;
    Gpio(Gpio && other) = delete;
    ~Gpio(){};


    static Gpio & null();

    __fast_inline void set(){
        instance_->BSHR = uint16_t(pin_);
    }
    __fast_inline void clr(){
        instance_->BCR = uint16_t(pin_);
    }
    __fast_inline void write(const bool val){(val) ? instance_->BSHR = uint16_t(pin_) : instance_->BCR = uint16_t(pin_);}
    __fast_inline bool read() const {return bool(instance_->INDR & uint16_t(pin_));}

    void setMode(const GpioMode mode) ;
    __fast_inline GPIO_TypeDef * inst() const {return instance_;} 
    __fast_inline int8_t index() const {
        if(likely(uint16_t(pin_)))
            return CTZ(uint16_t(pin_));
        else return -1;
    }
    template<hal::GpioTags::PortSource port_source,hal::GpioTags::PinSource pin_source>
    static constexpr Gpio reflect(){
        GPIO_TypeDef * _instance = GPIOC;

        return Gpio(
            _instance, 
            Pin(1 << uint8_t(pin_source))
        );
    }
};


extern Gpio & NullGpio;
}