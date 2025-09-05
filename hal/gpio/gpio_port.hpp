#pragma once

#include "gpio.hpp"
#include "gpio_port_intf.hpp"

namespace ymd::hal{
class GpioPort final: public GpioPortIntf{
protected:
    GPIO_TypeDef * inst_;
    Gpio channels[16];

    friend class Gpio;
public:
    GpioPort(GPIO_TypeDef * _instance):
        inst_(_instance),
        channels{
            Gpio(inst_, PinNth::_0),
            Gpio(inst_, PinNth::_1),
            Gpio(inst_, PinNth::_2),
            Gpio(inst_, PinNth::_3),
            Gpio(inst_, PinNth::_4),
            Gpio(inst_, PinNth::_5),
            Gpio(inst_, PinNth::_6),
            Gpio(inst_, PinNth::_7),
            Gpio(inst_, PinNth::_8),
            Gpio(inst_, PinNth::_9),
            Gpio(inst_, PinNth::_10),
            Gpio(inst_, PinNth::_11),
            Gpio(inst_, PinNth::_12),
            Gpio(inst_, PinNth::_13),
            Gpio(inst_, PinNth::_14),
            Gpio(inst_, PinNth::_15),
        }{;}
    void init();
    void enable_rcc(const Enable en = EN);


    __inline void write_nth(const Nth nth, const BoolLevel data);
    __inline void set_by_mask(const PinMask mask);
    __inline void clr_by_mask(const PinMask mask);
    __inline void write_by_mask(const PinMask mask){
        inst_->OUTDR = mask.as_u16();}
    __inline PinMask read_mask(){
        return PinMask::from_u16(inst_->INDR);}

    Gpio & operator [](const Nth nth){
        return channels[nth.count() & 0b1111];
    };

    Gpio & operator [](const PinNth pin_nth){
        return channels[CTZ(uint16_t(pin_nth)) & 0b1111];
    };

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
extern GpioPort portA;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PA(){
    return portA[Nth(I)];
}
#endif

#ifdef ENABLE_GPIOB
extern GpioPort portB;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PB(){
    return portB[Nth(I)];
}
#endif

#ifdef ENABLE_GPIOC
extern GpioPort portC;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PC(){
    return portC[Nth(I)];
}
#endif

#ifdef ENABLE_GPIOD
extern GpioPort portD;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PD(){
    return portD[Nth(I)];
}
#endif

#ifdef ENABLE_GPIOE
extern GpioPort portE;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PE(){
    return portE[Nth(I)];
}
#endif

}
