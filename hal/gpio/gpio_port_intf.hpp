#pragma once


#include "gpio_utils.hpp"


namespace ymd::hal{
class GpioPortIntf{
public:
    virtual void write_by_index(const size_t index, const BoolLevel data);
    virtual BoolLevel read_by_index(const size_t index);
    virtual void set_by_index(const size_t index);
    virtual void clr_by_index(const size_t index);

    void set_pin(const Pin pin){set_by_mask(uint16_t(pin));}
    void clr_pin(const Pin pin){clr_by_mask(uint16_t(pin));}

    virtual void set_by_mask(const uint16_t mask) = 0;
    virtual void clr_by_mask(const uint16_t mask) = 0;
    virtual void write_by_mask(const uint16_t mask) = 0;
    virtual uint16_t read_mask() = 0;
    
    virtual void set_mode(const size_t index, const GpioMode mode) = 0;
};

__fast_inline void GpioPortIntf::set_by_index(const size_t index){
    if(index < 0) return;
    set_by_mask(1 << index);
}

__fast_inline void GpioPortIntf::clr_by_index(const size_t index){
    if(index < 0) return;
    clr_by_mask(1 << index);
}

__fast_inline void GpioPortIntf::write_by_index(const size_t index, const BoolLevel data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        set_by_mask(mask);
    }else{
        clr_by_mask(mask);
    }
}

__fast_inline BoolLevel GpioPortIntf::read_by_index(const size_t index){
    if(index < 0) return LOW;
    return BoolLevel::from(this->read_mask() & (1 << index));
};

};