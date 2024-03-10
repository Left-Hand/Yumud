#ifndef __PORT_HPP__

#define __PORT_HPP__

#include "src/platform.h"
#include "gpio_enums.hpp"

class PortBase{
public:
    __fast_inline virtual void writeByIndex(const int8_t index, const bool data);
    __fast_inline virtual bool readByIndex(const int8_t index);

    virtual void setBits(const uint16_t & data) = 0;
    virtual void set(const Pin & pin) = 0;
    __fast_inline void setByIndex(const int8_t index);
    virtual void clrBits(const uint16_t & data) = 0;
    virtual void clr(const Pin & pin) = 0;
    __fast_inline void clrByIndex(const int8_t index);
    virtual void setModeByIndex(const int8_t & index, const PinMode & mode) = 0;
    virtual PortBase & operator = (const uint16_t & data) = 0;

    virtual operator uint16_t() = 0;
};

__fast_inline void PortBase::setByIndex(const int8_t index){
    if(index < 0) return;
    setBits(1 << index);
}

__fast_inline void PortBase::clrByIndex(const int8_t index){
    if(index < 0) return;
    clrBits(1 << index);
}

__fast_inline void PortBase::writeByIndex(const int8_t index, const bool data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        setBits(mask);
    }else{
        clrBits(mask);
    }
}

__fast_inline bool PortBase::readByIndex(const int8_t index){
    if(index < 0) return false;
    return uint16_t(*this) & (1 << index);
};


class Port : public PortBase{
protected:
    GPIO_TypeDef * instance;
public:
    Port(GPIO_TypeDef * _instance):instance(_instance){;}
    void init();
    void enableRcc(const bool en = true);
    __fast_inline void writeByIndex(const int8_t index, const bool data) override;
    __fast_inline void setBits(const uint16_t & data) override;
    __fast_inline void set(const Pin & pin) override;
    __fast_inline void clrBits(const uint16_t & data) override;
    __fast_inline void clr(const Pin & pin) override;
    Port & operator = (const uint16_t & data) override {instance->OUTDR = data; return *this;}

    operator uint16_t(){return instance->INDR;}
    void setModeByIndex(const int8_t & index, const PinMode & mode) override;

    GPIO_TypeDef * getInstance(){return instance;}
};

__fast_inline void Port::writeByIndex(const int8_t index, const bool data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        setBits(mask);
    }else{
        clrBits(mask);
    }
}

__fast_inline void Port::setBits(const uint16_t & data){
    instance->BSHR = data;
}

__fast_inline void Port::set(const Pin & pin){
    instance->BSHR = (uint16_t)pin;
}

__fast_inline void Port::clrBits(const uint16_t & data){
    instance->BCR = data;
}

__fast_inline void Port::clr(const Pin & pin){
    instance->BCR = (uint16_t)pin;
}


#ifdef HAVE_GPIOA
extern Port portA;
#endif

#ifdef HAVE_GPIOB
extern Port portB;
#endif

#ifdef HAVE_GPIOC
extern Port portC;
#endif

#ifdef HAVE_GPIOD
extern Port portD;
#endif

#ifdef HAVE_GPIOE
extern Port portE;
#endif

#endif