#pragma once


#include "gpio_utils.hpp"


namespace yumud{
class PortConcept{
public:
    virtual void writeByIndex(const int index, const bool data);
    virtual bool readByIndex(const int index);
    virtual void setByIndex(const int index);
    virtual void clrByIndex(const int index);

    virtual void set(const Pin pin){set((uint16_t)pin);}
    virtual void clr(const Pin pin){clr((uint16_t)pin);}
    
    virtual void set(const uint16_t mask) = 0;
    virtual void clr(const uint16_t mask) = 0;
    
    virtual void setMode(const int index, const GpioMode mode) = 0;
    virtual PortConcept & operator = (const uint16_t data) = 0;

    virtual operator uint16_t() = 0;
};

__fast_inline void PortConcept::setByIndex(const int index){
    if(index < 0) return;
    set(1 << index);
}

__fast_inline void PortConcept::clrByIndex(const int index){
    if(index < 0) return;
    clr(1 << index);
}

__fast_inline void PortConcept::writeByIndex(const int index, const bool data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        set(mask);
    }else{
        clr(mask);
    }
}

__fast_inline bool PortConcept::readByIndex(const int index){
    if(index < 0) return false;
    return uint16_t(*this) & (1 << index);
};
};