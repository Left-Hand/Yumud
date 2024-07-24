#ifndef __PORT_VIRTUAL_HPP__

#define __PORT_VIRTUAL_HPP__

#include "port.hpp"
#include "gpio.hpp"
#include <memory>
#include <array>

template<int size>
class PortVirtualConcept : public PortConcept{
protected:
    bool isIndexValid(const uint8_t index){return (index >= 0 && index < size);}

    virtual void write(const uint16_t data) = 0;
    virtual uint16_t read() = 0;
public:

    operator uint16_t(){return read();}

    constexpr uint8_t length(){
        return size;
    }

    PortVirtualConcept & operator = (const uint16_t data) override {write(data); return *this;}
    virtual void setModeByIndex(const int8_t index, const PinMode mode) = 0;
};

template<int size>
class PortVirtual : public PortVirtualConcept<size>{
protected:
    std::array<GpioConcept *, size> pin_ptrs = {nullptr};

    void write(const uint16_t data) override {
        for(uint8_t i = 0; i < 16; i++){
            writeByIndex(i, bool(data & (1 << i)));
        }
    }
    uint16_t read() override {
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(pin_ptrs[i]->read() << i);
        }
        return data;
    }
public:
    PortVirtual(){;}
    void init(){;}

    void bindPin(GpioConcept & gpio, const uint8_t index){
        if(index < 0 && index >= size)return;
        pin_ptrs[index] = &gpio;
    }

    void writeByIndex(const int8_t index, const bool data) override{
        if(!isIndexValid(index)) return;
        pin_ptrs[index]->write(data);
    }
    bool readByIndex(const int8_t index) override{
        if(!isIndexValid(index))return false;
        return bool(*(pin_ptrs[index]));
    }

    void setBits(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->set();
        }
    }
    void clrBits(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->clr();
        }
    }

    void set(const Pin pin) override{
        pin_ptrs[CTZ((uint16_t)pin)]->set();
    }
    void clr(const Pin pin) override{
        pin_ptrs[CTZ((uint16_t)pin)]->clr();
    }


    bool isIndexValid(const uint8_t index){return (index >= 0 && index < size && pin_ptrs[index] != nullptr);}

    GpioConcept & operator [](const uint8_t index){return isIndexValid(index) ? *pin_ptrs[index] : GpioNull;}

    void setModeByIndex(const int8_t index, const PinMode mode) override{
        if(!isIndexValid(index))return;
        pin_ptrs[index]->setMode(mode);
    }
};

template<int size>
class PortVirtualLocal : public PortVirtualConcept<size>{
protected:
    std::array<Gpio *, size> pin_ptrs = {nullptr};

    void write(const uint16_t & data) override {
        for(uint8_t i = 0; i < 16; i++){
            writeByIndex(i, bool(data & (1 << i)));
        }
    }
    uint16_t read() override {
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(pin_ptrs[i]->read() << i);
        }
        return data;
    }
public:
    PortVirtualLocal(){;}
    void init(){;}


    void bindPin(Gpio & gpio, const uint8_t index){
        if(index < 0 && index >= size)return;
        pin_ptrs[index] = &(gpio);
    }

    void writeByIndex(const int8_t index, const bool data) override{
        if(!isIndexValid(index)) return;
        pin_ptrs[index]->write(data);
    }
    bool readByIndex(const int8_t index) override{
        if(!isIndexValid(index))return false;
        return bool(*(pin_ptrs[index]));
    }

    void setBits(const uint16_t & data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->set();
        }
    }
    void clrBits(const uint16_t & data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->clr();
        }
    }

    void set(const Pin & pin) override{
        pin_ptrs[CTZ((uint16_t)pin)]->set();
    }
    void clr(const Pin & pin) override{
        pin_ptrs[CTZ((uint16_t)pin)]->clr();
    }


    bool isIndexValid(const uint8_t & index){return (index >= 0 && index < size && pin_ptrs[index] != nullptr);}

    Gpio * operator [](const uint8_t index){return isIndexValid(index) ? pin_ptrs[index].get() : nullptr;}

    void setModeByIndex(const int8_t & index, const PinMode & mode) override{
        if(!isIndexValid(index))return;
        pin_ptrs[index]->setMode(mode);
    }
};

#endif