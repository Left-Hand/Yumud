#pragma once

#include "gpio_intf.hpp"
#include "gpio_tag.hpp"

#include "core/sdk.hpp"

namespace ymd::hal{

class Exti;

class Gpio;

Gpio make_gpio(GPIO_TypeDef * inst, const Nth nth);

class Gpio final: public GpioIntf{
public:
    Gpio(const Gpio & other) = delete;
    Gpio(Gpio && other) = delete;
    ~Gpio(){};

    __fast_inline void set(){
        inst_->BSHR = static_cast<uint16_t>(pin_nth_);
    }
    __fast_inline void clr(){
        inst_->BCR = static_cast<uint16_t>(pin_nth_);
    }

    //BSHR的寄存器在BCR前 {1->BSHR; 0->BCR} 使用逻辑操作而非判断以提高速度
    __fast_inline void write(const BoolLevel val){
        *(&inst_->BCR - int(val.to_bool())) = static_cast<uint16_t>(pin_nth_);}

    __fast_inline BoolLevel read() const {
        return BoolLevel::from(inst_->INDR & static_cast<uint16_t>(pin_nth_));}

    hal::Gpio & operator = (const BoolLevel level){
        write(level);
        return *this;
    }

    void set_mode(const GpioMode mode) ;
    __fast_inline GPIO_TypeDef * inst() const {return inst_;} 
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

    PortSource port() const ;
private:
    GPIO_TypeDef * inst_;
    const PinNth pin_nth_;


    Gpio(GPIO_TypeDef * inst, const PinNth pin_nth);

    friend class VGpio;
    friend class ExtiChannel;
    friend class GpioPort;
    friend Gpio make_gpio(GPIO_TypeDef * inst, const Nth nth);
};

__inline Gpio make_gpio(GPIO_TypeDef * inst, const Nth nth){
    return Gpio{inst, std::bit_cast<PinNth>(uint16_t(1u << nth.count()))};
}

}