#ifndef __PORT_VIRTUAL_HPP__

#define __PORT_VIRTUAL_HPP__

#include "port.hpp"
#include "gpio.hpp"
#include <memory>
#include <array>

template<int size>
class PortVirtual : public PortBase{
protected:
    std::array<std::unique_ptr<GpioVirtual>, size> pin_ptrs = {nullptr};

    void write(const uint16_t & data){
        for(uint8_t i = 0; i < 16; i++){
            writeByIndex(i, bool(data & (1 << i)));
        }
    }
    const uint16_t read(){
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(pin_ptrs[i]->read() << i);
        }
        return data;
    }
public:
    PortVirtual(){;}
    void init(){;}
    void writeByIndex(const int8_t index, const bool data) override{
        if(!isIndexValid(index)) return;
        pin_ptrs[index]->write(data);
    }
    bool readByIndex(const int8_t index) override{
        if(!isIndexValid(index))return false;
        return bool(*(pin_ptrs[index]));
    }
    void bindPin(const GpioVirtual & gpio, const uint8_t index){
        if(index < 0 && index >= size)return;
        pin_ptrs[index] = std::make_unique<GpioVirtual>(gpio);
    }

    void bindPin(const Gpio & gpio, const uint8_t index){
        if(index < 0 && index >= size)return;
        pin_ptrs[index] = std::make_unique<GpioVirtual>(gpio);
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

    PortVirtual & operator = (const uint16_t & data){write(data); return *this;}

    operator uint16_t(){return read();}
    GpioVirtual * operator [](const uint8_t index){return isIndexValid(index) ? pin_ptrs[index].get() : nullptr;}

    void setModeByIndex(const int8_t & index, const PinMode & mode){
        if(!isIndexValid(index))return;
        pin_ptrs[index]->setMode(mode);
    }

    constexpr uint8_t getSize(){
        return size;
    }
};

#endif