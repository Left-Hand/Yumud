#ifndef __GPIO_HPP__

#define __GPIO_HPP__

#include "../defines/comm_inc.h"

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
    explicit operator bool(){return(this->read());}
};

class Gpio:public GpioBase{
    protected:
        GPIO_TypeDef* base = GPIOA;
        uint16_t pin = 0;

    public:
        Gpio(GPIO_TypeDef* _base, uint16_t _pin):base(_base), pin((_base == GPIOC && MCU_V) ? (_pin>>13) : _pin){};
        ~Gpio(){};

        __fast_inline void set()override{base->BSHR = pin;}
        __fast_inline void clr()override{base->BCR = pin;}
        __fast_inline void write(const bool & val)override{(val) ? base->BSHR = pin : base->BCR = pin;}
        __fast_inline bool read() const override{return (bool)(base->INDR & pin);}
        __fast_inline Gpio & operator = (const bool _val) override {(_val) ? base->BSHR = pin : base->BCR = pin; return *this;}

};


class GpioImag:public GpioBase{
private:
    typedef void (*WriteCallback)(uint16_t, bool);
    typedef bool (*ReadCallback)(uint16_t);

    const uint16_t index;

    WriteCallback write_callback;
    ReadCallback read_callback;

public:
    GpioImag(const uint16_t & _index, WriteCallback _write_callback = nullptr, ReadCallback _read_callback = nullptr)
        : index(_index), write_callback(_write_callback), read_callback(_read_callback){;}

    void set() override {if(write_callback) write_callback(index, true);}
    void clr() override{if(write_callback) write_callback(index, false);}
    void write(const bool & val){if(write_callback) write_callback(index, val);}
    bool read() const override {return read_callback(index);}

    GpioImag & operator = (const bool _val) override {write(_val); return *this;}
};

#ifdef MCU_V
#undef MCU_V
#endif

#endif