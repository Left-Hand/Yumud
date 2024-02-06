#ifndef __GPIO_HPP__

#define __GPIO_HPP__

#include "../defines/comm_inc.h"
#include "stdint.h"

#ifndef MCU_V
#define MCU_V (((*(uint32_t *) 0x40022030) & 0x0F000000) == 0)
#endif

inline uint8_t LeastBit(uint32_t x){
    uint8_t i = 0;
    for(; (x & 0x01) != 0; x >>= 1) i++;
    return i;
}

class GpioBase{
public:
    virtual void set() = 0;
    virtual void clr() = 0;
    virtual void write(const bool & val) = 0;
    virtual bool read() const = 0;

    virtual GpioBase & operator = (const bool _val) = 0;
    explicit operator bool(){return(this->read());}

    virtual void OutPP() = 0;
    virtual void OutOD() = 0;
    virtual void OutAfPP() = 0;
    virtual void OutAfOD() = 0;
    virtual void InAnalog() = 0;
    virtual void InFloating() = 0;
    virtual void InPull() = 0;
};

class Gpio:public GpioBase{
    protected:
        volatile GPIO_TypeDef* base = GPIOA;
        uint16_t pin = 0;
        uint8_t pin_index = 0;
        uint32_t pin_mask = 0;
        volatile uint32_t * pin_cfg = nullptr;

        __fast_inline void reConfig(const uint8_t cfg){
            uint32_t tempreg = *pin_cfg;
            tempreg &= pin_mask;
            tempreg |= (cfg << ((pin_index % 8) * 4));
            *pin_cfg = tempreg;
        }
    public:
        Gpio(GPIO_TypeDef* _base,const uint16_t & _pin):
            base(_base), 
            pin(((_base == GPIOC) && MCU_V) ? (_pin - 13) : _pin),
            pin_index(LeastBit(pin)),
            pin_mask(~(0xf << ((pin_index % 8) * 4))),
            pin_cfg(pin_index >= 8 ? (&(base -> CFGHR)) : (&(base -> CFGLR))){;}

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
        __fast_inline void InPull() override {reConfig(0b1000);}
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
    void InPull() override {if(dir_callback) dir_callback(index, false);}
};

#ifdef MCU_V
#undef MCU_V
#endif

#endif