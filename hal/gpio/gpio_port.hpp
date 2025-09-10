#pragma once

#include "gpio.hpp"
#include "gpio_port_intf.hpp"

namespace ymd::hal{
class GpioPort final: public GpioPortIntf{
protected:
    GPIO_TypeDef * inst_;

    friend class Gpio;
public:
    GpioPort(GPIO_TypeDef * inst):
        inst_(inst){;}

    void init();
    void enable_rcc(const Enable en);


    __inline void write_nth(const Nth nth, const BoolLevel data);
    __inline void set_by_mask(const PinMask mask);
    __inline void clr_by_mask(const PinMask mask);
    __inline void write_by_mask(const PinMask mask){
        inst_->OUTDR = mask.as_u16();}
    __inline PinMask read_mask(){
        return PinMask::from_u16(inst_->INDR);}


    void set_mode(const Nth nth, const GpioMode mode);
};

__inline void GpioPort::write_nth(const Nth nth, const BoolLevel data){
    const auto mask = PinMask::from_nth(nth);
    if(data == HIGH){
        set_by_mask(mask);
    }else{
        clr_by_mask(mask);
    }
}

__inline void GpioPort::set_by_mask(const PinMask mask){
    inst_->BSHR = mask.as_u16();
}

__inline void GpioPort::clr_by_mask(const PinMask mask){
    inst_->BCR = mask.as_u16();
}



#ifdef ENABLE_GPIOA
// extern GpioPort portA;
template<size_t I>
requires (I < 16)
static constexpr Gpio PA(){
    return make_gpio(GPIOA, Nth(I));
}
#endif

#ifdef ENABLE_GPIOB
// extern GpioPort portB;
template<size_t I>
requires (I < 16)
static constexpr Gpio PB(){
    return make_gpio(GPIOB, Nth(I));
}
#endif

#ifdef ENABLE_GPIOC
// extern GpioPort portC;
template<size_t I>
requires (I < 16)
static constexpr Gpio PC(){
    return make_gpio(GPIOC, Nth(I));
}
#endif

#ifdef ENABLE_GPIOD
// extern GpioPort portD;
template<size_t I>
requires (I < 16)
static constexpr Gpio PD(){
    return make_gpio(GPIOD, Nth(I));
}
#endif

#ifdef ENABLE_GPIOE
// extern GpioPort portE;
template<size_t I>
requires (I < 16)
static constexpr Gpio PE(){
    return make_gpio(GPIOE, Nth(I));
}
#endif

}
