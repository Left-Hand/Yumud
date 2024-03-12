#ifndef __GPIO_HPP__

#define __GPIO_HPP__

#include "bitband.h"
#include "port.hpp"
#include "gpio_enums.hpp"

#ifndef MCU_V
#define MCU_V (((*(uint32_t *) 0x40022030) & 0x0F000000) == 0)
#endif

class GpioConcept{
protected:
    const int8_t pin_index = 0;
public:
    GpioConcept(int8_t _pin_index):pin_index(_pin_index){;}
    virtual void set() = 0;
    virtual void clr() = 0;
    virtual void write(const bool & val) = 0;
    virtual bool read() const = 0;

    virtual GpioConcept & operator = (const bool _val) = 0;
    operator bool() const {return(this->read());}
    virtual void setMode(const PinMode mode) = 0;
    void OutPP(){setMode(PinMode::OutPP);}
    void OutOD(){setMode(PinMode::OutOD);}
    void OutAfPP(){setMode(PinMode::OutAfPP);}
    void OutAfOD(){setMode(PinMode::OutAfOD);}
    void InAnalog(){setMode(PinMode::InAnalog);}
    void InFloating(){setMode(PinMode::InFloating);}
    void InPullUP(){setMode(PinMode::InPullUP);}
    void InPullDN(){setMode(PinMode::InPullDN);}

    bool isValid() const {return pin_index >= 0;}
    int8_t getIndex() const {return pin_index;}
};

class Exti;

class Gpio:public GpioConcept{
protected:
    volatile GPIO_TypeDef * instance = GPIOA;
    const uint16_t pin;
    const uint32_t pin_mask = 0;
    volatile uint32_t & pin_cfg;

    friend class GpioVirtual;
    friend class Exti;
public:
    Gpio(GPIO_TypeDef * _instance,const Pin _pin):
        GpioConcept((_pin != Pin::None) ? CTZ((uint16_t)_pin) : -1),
        instance(_instance),
        pin(((_instance == GPIOC) && MCU_V) ? (((uint16_t)_pin >> 13)) : (uint16_t)_pin),
        pin_mask(~(0xf << ((CTZ(pin) % 8) * 4))),
        pin_cfg(CTZ(pin) >= 8 ? ((instance -> CFGHR)) : ((instance -> CFGLR))){
        }

    Gpio(Port & _port, const Pin _pin):Gpio(_port.getInstance(), _pin){;}
    ~Gpio(){};

    __fast_inline void set()override{instance->BSHR = pin;}
    __fast_inline void clr()override{instance->BCR = pin;}
    __fast_inline void write(const bool & val)override{(val) ? instance->BSHR = pin : instance->BCR = pin;}
    __fast_inline bool read() const override{return (bool)(instance->INDR & pin);}
    __fast_inline Gpio & operator = (const bool _val) override {(_val) ? instance->BSHR = pin : instance->BCR = pin; return *this;}
    __fast_inline Gpio & operator = (const Gpio & other){(other.read()) ? instance->BSHR = pin : instance->BCR = pin; return *this;}
    void setMode(const PinMode mode) override{
        if(!isValid()) return;
        uint32_t tempreg = pin_cfg;
        tempreg &= pin_mask;
        tempreg |= ((uint8_t)mode << ((pin_index % 8) * 4));
        pin_cfg = tempreg;

        if(mode == PinMode::InPullUP){
            instance -> OUTDR |= pin;
        }else if(mode == PinMode::InPullDN){
            instance -> OUTDR &= ~pin;
        }
    }
};

class GpioVirtual:public GpioConcept{
protected:
    PortBase * instance;

    PortBase * form_gpiotypedef_to_port(volatile GPIO_TypeDef * _instance);
public:
    GpioVirtual(const Gpio & gpio):GpioConcept(gpio.pin_index), instance(form_gpiotypedef_to_port(gpio.instance)){;}
    GpioVirtual(PortBase * _instance, const int8_t _pin_index):GpioConcept(_pin_index), instance(_instance){;}
    __fast_inline void set() override {instance->setByIndex(pin_index);}
    __fast_inline void clr() override {instance->clrByIndex(pin_index);}
    __fast_inline void write(const bool & val){instance->writeByIndex(pin_index, val);}
    __fast_inline bool read() const override {return instance->readByIndex(pin_index);}

    __fast_inline GpioVirtual & operator = (const bool _val) override {write(_val); return *this;}
    __fast_inline GpioVirtual & operator = (GpioConcept & other) {write(other.read()); return *this;}
    void setMode(const PinMode mode) override{instance->setModeByIndex(pin_index, mode);}
};


#ifdef MCU_V
#undef MCU_V
#endif

#endif