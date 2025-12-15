#pragma once

#include "gpio.hpp"
#include "gpio_port_intf.hpp"

namespace ymd::hal{
class GpioPort final: public GpioPortIntf{
protected:
    void * inst_;

    friend class Gpio;
public:
    GpioPort(void * inst):
        inst_(inst){;}

    void init();
    void enable_rcc(const Enable en);


    void write_nth(const Nth nth, const BoolLevel data);
    void set_by_mask(const PinMask mask);
    void clr_by_mask(const PinMask mask);
    void write_by_mask(const PinMask mask);
    PinMask read_mask();


    void set_mode(const Nth nth, const GpioMode mode);
};




#ifdef GPIOA_PRESENT
template<size_t I>
requires (I < 16)
static constexpr Gpio PA(){
    return make_gpio(PortSource::PA, Nth(I));
}
#endif

#ifdef GPIOB_PRESENT
template<size_t I>
requires (I < 16)
static constexpr Gpio PB(){
    return make_gpio(PortSource::PB, Nth(I));
}
#endif

#ifdef GPIOC_PRESENT
template<size_t I>
requires (I < 16)
static constexpr Gpio PC(){
    return make_gpio(PortSource::PC, Nth(I));
}
#endif

#ifdef GPIOD_PRESENT
template<size_t I>
requires (I < 16)
static constexpr Gpio PD(){
    return make_gpio(PortSource::PD, Nth(I));
}
#endif

#ifdef GPIOE_PRESENT
template<size_t I>
requires (I < 16)
static constexpr Gpio PE(){
    return make_gpio(PortSource::PE, Nth(I));
}
#endif

}
