#ifndef __PORT_HPP__

#define __PORT_HPP__

#include "gpio_enums.h"
#include "src/platform.h"
#include "gpio.hpp"


class GpioVirtual;

class PortBase{
public:
    virtual void writeByIndex(const int8_t & index, const bool & data){
        if(index < 0) return;
        uint16_t mask = 1 << index;
        if(data){
            setBits(mask);
        }else{
            clrBits(mask);
        }
    }
    virtual bool readByIndex(const int8_t & index){
        if(index < 0) return false;
        return uint16_t(*this) & (1 << index);
    };

    virtual void setBits(const uint16_t & data) = 0;
    virtual void set(const Pin & pin) = 0;
    void setByIndex(const int8_t index);
    virtual void clrBits(const uint16_t & data) = 0;
    virtual void clr(const Pin & pin) = 0;
    void clrByIndex(const int8_t index);
    virtual void setModeByIndex(const int8_t & index, const PinMode & mode) = 0;
    virtual PortBase & operator = (const uint16_t & data) = 0;

    virtual operator uint16_t() = 0;
};

class Port : public PortBase{
protected:
    GPIO_TypeDef * instance;
public:
    Port(GPIO_TypeDef * _instance):instance(_instance){;}
    void init();
    void enableRcc(const bool en = true);

    void setBits(const uint16_t & data) override;
    void set(const Pin & pin) override;
    void clrBits(const uint16_t & data) override;
    void clr(const Pin & pin) override;
    Port & operator = (const uint16_t & data) override {instance->OUTDR = data; return *this;}

    operator uint16_t(){return instance->INDR;}
    void setModeByIndex(const int8_t & index, const PinMode & mode) override{
        Gpio gpio = Gpio(instance, (Pin)(1 << index));
        gpio.setMode(mode);
    }
};

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