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


    __inline void write_nth(const size_t index, const BoolLevel data) override;
    __inline void set_by_mask(const PinMask mask) override;
    __inline void clr_by_mask(const PinMask mask) override;
    __inline void write_by_mask(const PinMask mask){
        inst_->OUTDR = mask.as_u16();}
    __inline PinMask read_mask(){
        return PinMask(inst_->INDR);}

    Gpio & operator [](const size_t index){
        return channels[index & 0b1111];
    };

    Gpio & operator [](const PinNth pin_nth){
        return channels[CTZ(uint16_t(pin_nth)) & 0b1111];
    };

    void set_mode(const size_t index, const GpioMode mode) override;
};

__inline void GpioPort::write_nth(const size_t index, const BoolLevel data){
    const auto mask = PinMask::from_nth(index);
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
    return portA[I];
}
#endif

#ifdef ENABLE_GPIOB
extern GpioPort portB;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PB(){
    return portB[I];
}
#endif

#ifdef ENABLE_GPIOC
extern GpioPort portC;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PC(){
    return portC[I];
}
#endif

#ifdef ENABLE_GPIOD
extern GpioPort portD;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PD(){
    return portD[I];
}
#endif

#ifdef ENABLE_GPIOE
extern GpioPort portE;
template<size_t I>
requires (I < 16)
static constexpr Gpio & PE(){
    return portE[I];
}
#endif

}
