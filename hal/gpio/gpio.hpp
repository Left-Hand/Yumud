#pragma once

#include "gpio_intf.hpp"
#include "gpio_tag.hpp"

#include "core/sdk.hpp"

namespace ymd::hal{

class Exti;

class Gpio final: public GpioIntf{
protected:
    GPIO_TypeDef * instance_;
    const PinNth pin_nth_;


    Gpio(GPIO_TypeDef * instance, const PinNth pin):
        instance_(instance)

        #if defined(CH32V20X) || defined(CH32V30X)
        ,pin_nth_(PinNth(
            (instance == GPIOC) && 
            (
                ((* reinterpret_cast<uint32_t *> (0x40022030) & 0x0F000000) == 0)//MCU version for wch mcu, see wch sdk
            ) ? uint16_t(uint16_t(pin) >> 13) : uint16_t(pin)
        ))
        #elif defined(USE_STM32_HAL_LIB)
        ,pin_nth_(pin)
        #endif
        {}

    friend class VGpio;
    friend class ExtiChannel;
    friend class GpioPort;


public:
    using GpioIntf::operator=;

    Gpio(const Gpio & other) = delete;
    Gpio(Gpio && other) = delete;
    ~Gpio(){};

    __fast_inline void set(){
        instance_->BSHR = static_cast<uint16_t>(pin_nth_);
    }
    __fast_inline void clr(){
        instance_->BCR = static_cast<uint16_t>(pin_nth_);
    }

    //BSHR的寄存器在BCR前 {1->BSHR; 0->BCR} 使用逻辑操作而非判断以提高速度
    __fast_inline void write(const BoolLevel val){
        *(&instance_->BCR - int(val.to_bool())) = static_cast<uint16_t>(pin_nth_);}

    __fast_inline BoolLevel read() const {
        return BoolLevel::from(instance_->INDR & static_cast<uint16_t>(pin_nth_));}

    hal::Gpio & operator = (const BoolLevel level){
        write(level);
        return *this;
    }

    hal::Gpio & operator = (const hal::Gpio & other){
        write(other.read());
        return *this;
    }


    void set_mode(const GpioMode mode) ;
    __fast_inline GPIO_TypeDef * inst() const {return instance_;} 
    __fast_inline Nth nth() const {
        return Nth(CTZ(static_cast<uint16_t>(pin_nth_)));
    }


    // template<hal::GpioTags::PortSource port_source,hal::GpioTags::PinNth pin_source>
    // static constexpr Gpio reflect(){
    //     GPIO_TypeDef * _instance = GPIOC;

    //     return Gpio(
    //         _instance, 
    //         PinNth(1 << uint8_t(pin_source))
    //     );
    // }

    constexpr PinNth pin_nth() const {return pin_nth_;}

    constexpr PortSource port() const {
        const auto base = reinterpret_cast<uint32_t>(instance_);
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
};
}