#ifndef __GPIO_HPP__

#define __GPIO_HPP__

#include "src/comm_inc.h"
#include "stdint.h"

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
    operator bool(){return(this->read());}

    virtual void OutPP() = 0;
    virtual void OutOD() = 0;
    virtual void OutAfPP() = 0;
    virtual void OutAfOD() = 0;
    virtual void InAnalog() = 0;
    virtual void InFloating() = 0;
    virtual void InPullUP() = 0;
    virtual void InPullDN() = 0;
};

class Gpio:public GpioBase{
protected:
    volatile GPIO_TypeDef* base = GPIOA;
    uint16_t pin = 0;
    uint8_t pin_index = 0;
    uint32_t pin_mask = 0;
    volatile uint32_t & pin_cfg;

    __fast_inline void reConfig(const uint8_t cfg){
        uint32_t tempreg = pin_cfg;
        tempreg &= pin_mask;
        tempreg |= (cfg << ((pin_index % 8) * 4));
        pin_cfg = tempreg;
    }
public:
    Gpio(GPIO_TypeDef* _base,const uint16_t & _pin):
        base(_base),
        pin(((_base == GPIOC) && MCU_V) ? (_pin >> 13) : _pin),
        pin_index(__builtin_ctz(pin)),
        pin_mask(~(0xf << ((pin_index % 8) * 4))),
        pin_cfg(pin_index >= 8 ? ((base -> CFGHR)) : ((base -> CFGLR))){;}

    ~Gpio(){};

    __fast_inline void set()override{base->BSHR = pin;}
    __fast_inline void clr()override{base->BCR = pin;}
    __fast_inline void write(const bool & val)override{(val) ? base->BSHR = pin : base->BCR = pin;}
    __fast_inline bool read() const override{return (bool)(base->INDR & pin);}
    __fast_inline Gpio & operator = (const bool _val) override {(_val) ? base->BSHR = pin : base->BCR = pin; return *this;}
    __fast_inline Gpio & operator = (const Gpio & other){(other.read()) ? base->BSHR = pin : base->BCR = pin; return *this;}
    __fast_inline void OutPP() override {reConfig(0b0011);}
    __fast_inline void OutOD() override {reConfig(0b0111);}
    __fast_inline void OutAfPP() override {reConfig(0b1011);}
    __fast_inline void OutAfOD() override {reConfig(0b1111);}
    __fast_inline void InAnalog() override {reConfig(0b0000);}
    __fast_inline void InFloating() override {reConfig(0b0100);}
    __fast_inline void InPullUP() override {reConfig(0b1000); base -> OUTDR |= pin;}
    __fast_inline void InPullDN() override {reConfig(0b1100); base -> OUTDR &= ~pin;}
};


class GpioImag:public GpioBase{
private:
    typedef void (*WriteCallback)(uint16_t, bool);
    typedef bool (*ReadCallback)(uint16_t);
    typedef void (*DirCallback)(uint16_t, bool);

    uint16_t index;

    WriteCallback write_callback;
    ReadCallback read_callback;
    DirCallback dir_callback;
public:
    GpioImag(const uint16_t & _index, WriteCallback _write_callback = nullptr, 
        ReadCallback _read_callback = nullptr,DirCallback _dir_callback = nullptr)
        : index(_index), write_callback(_write_callback), read_callback(_read_callback), dir_callback(_dir_callback){;}

    void set() override {if(write_callback) write_callback(index, true);}
    void clr() override{if(write_callback) write_callback(index, false);}
    void write(const bool & val){if(write_callback) write_callback(index, val);}
    bool read() const override {return read_callback ? read_callback(index) : false;}

    GpioImag & operator = (const bool _val) override {write(_val); return *this;}
    GpioImag & operator = (GpioImag & other) {write(other.read()); return *this;}

    void OutPP() override {if(dir_callback) dir_callback(index, true);}
    void OutOD() override {if(dir_callback) dir_callback(index, true);}
    void OutAfPP() override {if(dir_callback) dir_callback(index, true);}
    void OutAfOD() override {if(dir_callback) dir_callback(index, true);}
    void InAnalog() override {if(dir_callback) dir_callback(index, false);}
    void InFloating() override {if(dir_callback) dir_callback(index, false);}
    void InPullUP() override {if(dir_callback) dir_callback(index, false);}
    void InPullDN() override {if(dir_callback) dir_callback(index, false);}
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