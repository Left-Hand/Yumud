#pragma once

#include "gpio.hpp"
#include "port_concept.hpp"

namespace ymd{
class Port : public PortConcept{
protected:
    GPIO_TypeDef * instance;
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
    __inline void writeByIndex(const int index, const bool data) override;
    __inline void setPin(const uint16_t data) override;
    __inline void setPin(const Pin pin) override;
    __inline void clrPin(const uint16_t data) override;
    __inline void clrPin(const Pin pin) override;
    Port & operator = (const uint16_t data) override {instance->OUTDR = data; return *this;}

    operator uint16_t(){return instance->INDR;}

    Gpio & operator [](const int index){
        if(index < 0) return Gpio::null();
        else return channels[index & 0b1111];
    };
    Gpio & operator [](const Pin pin){
        if(pin != Pin::None) return channels[CTZ((uint16_t)pin) & 0b1111];
        else return Gpio::null();
    };

    void setMode(const int index, const GpioMode mode) override;

};

__inline void Port::writeByIndex(const int index, const bool data){
    if(index < 0) return;
    uint16_t mask = 1 << index;
    if(data){
        setPin(mask);
    }else{
        clrPin(mask);
    }
}

__inline void Port::setPin(const uint16_t data){
    instance->BSHR = data;
}

__inline void Port::setPin(const Pin pin){
    instance->BSHR = (uint16_t)pin;
}

__inline void Port::clrPin(const uint16_t data){
    instance->BCR = data;
}

__inline void Port::clrPin(const Pin pin){
    instance->BCR = (uint16_t)pin;
}


#ifdef ENABLE_GPIOA
extern Port portA;
#endif

#ifdef ENABLE_GPIOB
extern Port portB;
#endif

#ifdef ENABLE_GPIOC
extern Port portC;
#endif

#ifdef ENABLE_GPIOD
extern Port portD;
#endif

#ifdef ENABLE_GPIOE
extern Port portE;
#endif

}
