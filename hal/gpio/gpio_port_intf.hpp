#pragma once


#include "gpio_utils.hpp"


namespace ymd::hal{
class GpioPortIntf{
public:
    virtual void write_by_index(const int index, const bool data);
    virtual bool read_by_index(const int index);
    virtual void set_by_index(const int index);
    virtual void clr_by_index(const int index);

    void set_pin(const Pin pin){set_pin(uint16_t(pin));}
    void clr_pin(const Pin pin){clr_pin(uint16_t(pin));}

    virtual void set_pin(const uint16_t mask) = 0;
    virtual void clr_pin(const uint16_t mask) = 0;
    
    virtual void set_mode(const int index, const GpioMode mode) = 0;

    virtual GpioPortIntf & operator = (const uint16_t data) = 0;
    virtual explicit operator uint16_t() = 0;
};

__fast_inline void GpioPortIntf::set_by_index(const int index){
    if(index < 0) return;
    set_pin(1 << index);
}

__fast_inline void GpioPortIntf::clr_by_index(const int index){
    if(index < 0) return;
    clr_pin(1 << index);
}

__fast_inline void GpioPortIntf::write_by_index(const int index, const bool data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        set_pin(mask);
    }else{
        clr_pin(mask);
    }
}

__fast_inline bool GpioPortIntf::read_by_index(const int index){
    if(index < 0) return false;
    return uint16_t(*this) & (1 << index);
};

};