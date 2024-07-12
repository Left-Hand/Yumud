#ifndef __PORT_HPP__

#define __PORT_HPP__

#include "sys/core/platform.h"
#include "gpio.hpp"
#include "port_concept.hpp"
#include "gpio_enums.hpp"

class Gpio;

class Port : public PortConcept{
protected:
    GPIO_TypeDef * instance;
    static Gpio channel_none;
    Gpio channels[16];

    friend class Gpio;
public:
    Port(GPIO_TypeDef * _instance):
        instance(_instance),
        channels{
            Gpio(instance, Pin::_0),
            Gpio(instance, Pin::_1),
            Gpio(instance, Pin::_2),
            Gpio(instance, Pin::_3),
            Gpio(instance, Pin::_4),
            Gpio(instance, Pin::_5),
            Gpio(instance, Pin::_6),
            Gpio(instance, Pin::_7),
            Gpio(instance, Pin::_8),
            Gpio(instance, Pin::_9),
            Gpio(instance, Pin::_10),
            Gpio(instance, Pin::_11),
            Gpio(instance, Pin::_12),
            Gpio(instance, Pin::_13),
            Gpio(instance, Pin::_14),
            Gpio(instance, Pin::_15),
        }{;}
    void init();
    void enableRcc(const bool en = true);
    __fast_inline void writeByIndex(const int8_t index, const bool data) override;
    // __fast_inline bool readByIndex(const int8_t index) override;
    __fast_inline void setBits(const uint16_t & data) override;
    // __fast_inline void setByIndex(const int8_t index) override;
    __fast_inline void set(const Pin & pin) override;
    __fast_inline void clrBits(const uint16_t & data) override;
    __fast_inline void clr(const Pin & pin) override;
    // __fast_inline void clrByIndex(const int8_t index) override;
    Port & operator = (const uint16_t & data) override {instance->OUTDR = data; return *this;}

    operator uint16_t(){return instance->INDR;}
    Gpio & operator [](const int index){
        if(index < 0) return channel_none;
        else return channels[CLAMP(index, 0, 15)];
    };
    Gpio & operator [](const Pin pin){
        if(pin != Pin::None) return channels[CLAMP(CTZ((uint16_t)pin), 0, 15)];
        else return channel_none;
    };
    void setModeByIndex(const int8_t & index, const PinMode & mode) override;

    // GPIO_TypeDef * getInstance(){return instance;}
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


#define GpioNull portD[Pin::None]

#endif