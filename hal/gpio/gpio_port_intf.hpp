#pragma once


#include "gpio_utils.hpp"


namespace ymd::hal{
class GpioPortIntf{
public:
    virtual void write_nth(const Nth nth, const BoolLevel data);
    virtual BoolLevel read_nth(const Nth nth);
    virtual void set_nth(const Nth nth);
    virtual void clr_nth(const Nth nth);

    void set_pin(const PinNth pin){
        set_by_mask(PinMask(pin));}
    void clr_pin(const PinNth pin){
        clr_by_mask(PinMask(pin));}

    virtual void set_by_mask(const PinMask mask) = 0;
    virtual void clr_by_mask(const PinMask mask) = 0;
    virtual void write_by_mask(const PinMask mask) = 0;
    virtual PinMask read_mask() = 0;
    
    virtual void set_mode(const Nth nth, const GpioMode mode) = 0;
};

__fast_inline void GpioPortIntf::set_nth(const Nth nth){
    set_by_mask(PinMask::from_nth(nth));
}

__fast_inline void GpioPortIntf::clr_nth(const Nth nth){
    clr_by_mask(PinMask::from_nth(nth));
}

__fast_inline void GpioPortIntf::write_nth(
    const Nth nth, 
    const BoolLevel data)
{
    const auto mask = PinMask::from_nth(nth);
    if(data == HIGH){
        set_by_mask(mask);
    }else{
        clr_by_mask(mask);
    }
}

__fast_inline BoolLevel GpioPortIntf::read_nth(const Nth nth){
    return this->read_mask().test(nth) ? HIGH : LOW;
};

};