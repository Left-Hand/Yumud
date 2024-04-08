#ifndef __PORT_CONCEPT_HPP__

#define __PORT_CONCEPT_HPP__

#include "src/platform.h"
#include "gpio_enums.hpp"

class PortConcept{
public:
    virtual void writeByIndex(const int8_t index, const bool data);
    virtual bool readByIndex(const int8_t index);

    virtual void setBits(const uint16_t & data) = 0;
    virtual void set(const Pin & pin) = 0;
    virtual void setByIndex(const int8_t index);
    virtual void clrBits(const uint16_t & data) = 0;
    virtual void clr(const Pin & pin) = 0;
    virtual void clrByIndex(const int8_t index);
    virtual void setModeByIndex(const int8_t & index, const PinMode & mode) = 0;
    virtual PortConcept & operator = (const uint16_t & data) = 0;

    virtual operator uint16_t() = 0;
};

__fast_inline void PortConcept::setByIndex(const int8_t index){
    if(index < 0) return;
    setBits(1 << index);
}

__fast_inline void PortConcept::clrByIndex(const int8_t index){
    if(index < 0) return;
    clrBits(1 << index);
}

__fast_inline void PortConcept::writeByIndex(const int8_t index, const bool data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        setBits(mask);
    }else{
        clrBits(mask);
    }
}

__fast_inline bool PortConcept::readByIndex(const int8_t index){
    if(index < 0) return false;
    return uint16_t(*this) & (1 << index);
};
#endif