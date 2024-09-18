#ifndef __GPIO_HPP__

#define __GPIO_HPP__

#include "bitband.h"
#include "port_concept.hpp"
#include "gpio_enums.hpp"

class GpioConcept{
public:
    const int8_t pin_index = 0;
public:
    GpioConcept(int8_t _pin_index):pin_index(_pin_index){;}
    virtual void set() = 0;
    virtual void clr() = 0;
    virtual void write(const bool val) = 0;
    virtual bool read() const = 0;
    bool toggle() {bool val = !this->read(); write(val); return val;}

    virtual GpioConcept & operator = (const bool _val) = 0;
    operator bool() const {return(this->read());}
    virtual void setMode(const PinMode mode) = 0;
    void outpp(){setMode(PinMode::OutPP);}
    void outod(){setMode(PinMode::OutOD);}
    void afpp(){setMode(PinMode::OutAfPP);}
    void afod(){setMode(PinMode::OutAfOD);}

    void outpp(const bool initial_state){setMode(PinMode::OutPP);write(initial_state);}
    void outod(const bool initial_state){setMode(PinMode::OutOD);write(initial_state);}
    void afpp(const bool initial_state){setMode(PinMode::OutAfPP);write(initial_state);}
    void afod(const bool initial_state){setMode(PinMode::OutAfOD);write(initial_state);}

    void inana(){setMode(PinMode::InAnalog);}
    void inflt(){setMode(PinMode::InFloating);}
    void inpu(){setMode(PinMode::InPullUP);}
    void inpd(){setMode(PinMode::InPullDN);}

    bool isValid() const {return pin_index >= 0;}
    int8_t getIndex() const {return pin_index;}
};

class Exti;

class Gpio:public GpioConcept{
protected:
    volatile GPIO_TypeDef * instance = nullptr;
    const uint16_t pin;
    const uint32_t pin_mask;

    volatile uint32_t * pin_cfg;

    Gpio(GPIO_TypeDef * _instance,const Pin _pin):
        GpioConcept((_pin != Pin::None) ? CTZ((uint16_t)_pin) : -1),
        instance(_instance),

        #if defined(CH32V20X) || defined(CH32V30X)
        pin(((_instance == GPIOC) && 
            (((*(uint32_t *) 0x40022030) & 0x0F000000) == 0)//MCU version for wch mcu, see wch sdk
            ) ? (((uint16_t)_pin >> 13)) : (uint16_t)_pin),
        #elif defined(USE_STM32_HAL_LIB)
        pin((uint16_t)_pin),
        #endif

        pin_mask(~(0xf << ((CTZ(pin) % 8) * 4))),
        pin_cfg(CTZ(pin) >= 8 ? &((instance -> CFGHR)) : &((instance -> CFGLR))){
    }

    friend class GpioVirtual;
    friend class ExtiChannel;
    friend class Port;
public:

    Gpio(const Gpio & other) = delete;
    Gpio(Gpio && other) = delete;

    ~Gpio(){};

    __fast_inline void set()override{
        instance->BSHR = pin;
    }
    __fast_inline void clr()override{
        instance->BCR = pin;
    }
    __fast_inline void write(const bool val)override{(val) ? instance->BSHR = pin : instance->BCR = pin;}
    __fast_inline bool read() const override{return (bool)(instance->INDR & pin);}
    __fast_inline volatile GPIO_TypeDef * inst() {return instance;} 
    __fast_inline Gpio & operator = (const bool _val) override {
        write(_val);
        return *this;
    }
    __fast_inline Gpio & operator = (const Gpio & other){
        write(other.read());
        return *this;
    }

    void setMode(const PinMode mode) override;
};

class GpioVirtual:public GpioConcept{
protected:
    PortConcept & instance;

    PortConcept & form_gpiotypedef_to_port(volatile GPIO_TypeDef * _instance);
public:
    GpioVirtual(const Gpio & gpio):GpioConcept(gpio.pin_index), instance(form_gpiotypedef_to_port(gpio.instance)){;}
    GpioVirtual(PortConcept & _instance, const int8_t _pin_index):GpioConcept(_pin_index), instance(_instance){;}
    GpioVirtual(PortConcept & _instance, const Pin _pin):GpioConcept(CTZ((uint16_t)_pin)), instance(_instance){;}
    __fast_inline void set() override {instance.setByIndex(pin_index);}
    __fast_inline void clr() override {instance.clrByIndex(pin_index);}
    __fast_inline void write(const bool val){instance.writeByIndex(pin_index, val);}
    __fast_inline bool read() const override {return instance.readByIndex(pin_index);}

    __fast_inline GpioVirtual & operator = (const bool _val) override {write(_val); return *this;}
    __fast_inline GpioVirtual & operator = (GpioConcept & other) {write(other.read()); return *this;}
    void setMode(const PinMode mode) override{instance.setMode(pin_index, mode);}
};

#endif