#ifndef __GPIO_HPP__

#define __GPIO_HPP__

#include "gpio_enums.h"
#include "src/platform.h"
#include "stdint.h"
#include <functional>

#ifndef MCU_V
#define MCU_V (((*(uint32_t *) 0x40022030) & 0x0F000000) == 0)
#endif


class GpioBase{
public:
    virtual void set() = 0;
    virtual void clr() = 0;
    virtual void write(const bool & val) = 0;
    virtual bool read() const = 0;

    virtual GpioBase & operator = (const bool _val) = 0;
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

    virtual bool isValid() const = 0;
};

class Gpio:public GpioBase{
protected:
    volatile GPIO_TypeDef* instance = GPIOA;
    const Pin pin;
    int8_t pin_index = 0;
    uint32_t pin_mask = 0;
    volatile uint32_t & pin_cfg;



    friend class PortVirtual;
public:
    Gpio(GPIO_TypeDef * _instance,const Pin _pin):
        instance(_instance),
        pin(((_instance == GPIOC) && MCU_V) ? ((Pin)((uint16_t)_pin >> 13)) : _pin),
        pin_index(__builtin_ctz((uint16_t)pin)),
        pin_mask(~(0xf << ((pin_index % 8) * 4))),
        pin_cfg(pin_index >= 8 ? ((instance -> CFGHR)) : ((instance -> CFGLR))){;}

    ~Gpio(){};

    __fast_inline void set()override{instance->BSHR = pin;}
    __fast_inline void clr()override{instance->BCR = pin;}
    __fast_inline void write(const bool & val)override{(val) ? instance->BSHR = pin : instance->BCR = pin;}
    __fast_inline bool read() const override{return (bool)(instance->INDR & pin);}
    __fast_inline Gpio & operator = (const bool _val) override {(_val) ? instance->BSHR = pin : instance->BCR = pin; return *this;}
    __fast_inline Gpio & operator = (const Gpio & other){(other.read()) ? instance->BSHR = pin : instance->BCR = pin; return *this;}

    bool isValid() const {return pin != PinNone;}

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


class GpioVirtual:public GpioBase{
protected:
    typedef std::function<void(const int8_t&, const bool&)> WriteCallback;
    typedef std::function<bool(const int8_t&)> ReadCallback;
    typedef std::function<void(const int8_t&, const PinMode&)> ModeCallback;

    int8_t pin_index;

    WriteCallback write_callback;
    ReadCallback read_callback;
    ModeCallback mode_callback;

    friend class PortVirtual;
public:
    GpioVirtual(const int8_t & _pin_index, WriteCallback _write_callback = nullptr,
        ReadCallback _read_callback = nullptr, ModeCallback _mode_callback = nullptr)
        : pin_index(_pin_index), write_callback(_write_callback), read_callback(_read_callback), mode_callback(_mode_callback){;}

    void set() override {if(write_callback) write_callback(pin_index, true);}
    void clr() override{if(write_callback) write_callback(pin_index, false);}
    void write(const bool & val){if(write_callback) write_callback(pin_index, val);}
    bool read() const override {return read_callback ? read_callback(pin_index) : false;}

    GpioVirtual & operator = (const bool _val) override {write(_val); return *this;}
    GpioVirtual & operator = (GpioVirtual & other) {write(other.read()); return *this;}

    bool isValid() const override{return true;}

    void setMode(const PinMode mode) override{if(mode_callback) mode_callback(pin_index, mode);}
};

typedef struct {
	uint32_t bit0 :1;
	uint32_t bit1 :1;
	uint32_t bit2 :1;
	uint32_t bit3 :1;
	uint32_t bit4 :1;
	uint32_t bit5 :1;
	uint32_t bit6 :1;
	uint32_t bit7 :1;
	uint32_t bit8 :1;
	uint32_t bit9 :1;
	uint32_t bit10 :1;
	uint32_t bit11 :1;
	uint32_t bit12 :1;
	uint32_t bit13 :1;
	uint32_t bit14 :1;
	uint32_t bit15 :1;
	uint32_t bit16 :1;
	uint32_t bit17 :1;
	uint32_t bit18 :1;
	uint32_t bit19 :1;
	uint32_t bit20 :1;
	uint32_t bit21 :1;
	uint32_t bit22 :1;
	uint32_t bit23 :1;
	uint32_t bit24 :1;
	uint32_t bit25 :1;
	uint32_t bit26 :1;
	uint32_t bit27 :1;
	uint32_t bit28 :1;
	uint32_t bit29 :1;
	uint32_t bit30 :1;
	uint32_t bit31 :1;
} GPIO_REG;

#define PAout(n)	(((GPIO_REG *)(&(GPIOA->OUTDR)))->bit##n)
#define PAin(n)		(((GPIO_REG *)(&(GPIOA->INDR)))->bit##n)
#define PBout(n)	(((GPIO_REG *)(&(GPIOB->OUTDR)))->bit##n)
#define PBin(n)		(((GPIO_REG *)(&(GPIOB->INDR)))->bit##n)
#define PCout(n)	(((GPIO_REG *)(&(GPIOC->OUTDR)))->bit##n)
#define PCin(n)		(((GPIO_REG *)(&(GPIOC->INDR)))->bit##n)

#ifdef MCU_V
#undef MCU_V
#endif

#endif