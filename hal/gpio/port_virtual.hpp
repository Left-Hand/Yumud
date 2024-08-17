#ifndef __PORT_VIRTUAL_HPP__

#define __PORT_VIRTUAL_HPP__

#include "port.hpp"
#include "gpio.hpp"
#include <memory>
#include <array>

template<size_t N>
class PortVirtualConcept : public PortConcept{
protected:
    bool isIndexValid(const size_t index){return (index < N);}

    virtual void write(const uint16_t data) = 0;
    virtual uint16_t read() = 0;
public:

    operator uint16_t(){return read();}

    constexpr size_t size(){
        return N;
    }

    PortVirtualConcept & operator = (const uint16_t data) override {write(data); return *this;}
    virtual void setMode(const int index, const PinMode mode) = 0;
};

template<size_t N>
class PortVirtual : public PortVirtualConcept<N>{
protected:
    using E = GpioConcept;
    std::array<E *, N> pin_ptrs = {nullptr};

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

    PortVirtual(const PortVirtual<N> & other){
        pin_ptrs = other.pin_ptrs;
    }

    PortVirtual(PortVirtual<N> && other){
        pin_ptrs = std::move(other.pin_ptrs);
    }
    void init(){;}

    void bindPin(GpioConcept & gpio, const size_t index){
        if(index >= N)return;
        pin_ptrs[index] = &gpio;
    }

    void writeByIndex(const int index, const bool data) override{
        if(!isIndexValid(index)) return;
        pin_ptrs[index]->write(data);
    }

    bool readByIndex(const int index) override{
        if(!isIndexValid(index))return false;
        return bool(*(pin_ptrs[index]));
    }

    void set(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->set();
        }
    }

    void clr(const uint16_t data) override{
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

    E * begin(){
        return pin_ptrs.front();
    }

    E * end(){
        return begin() + N;
    }

    bool isIndexValid(const size_t index){return (index < N and pin_ptrs[index] != nullptr);}

    E & operator [](const size_t index){return isIndexValid(index) ? *pin_ptrs[index] : GpioNull;}

    void setMode(const int index, const PinMode mode) override{
        if(index < 0) return;
        if(!isIndexValid(index))return;
        pin_ptrs[index]->setMode(mode);
    }
};

template<size_t N>
class PortVirtualLocal : public PortVirtualConcept<N>{
protected:
    using E = Gpio;
    std::array<E *, N> pin_ptrs = {nullptr};

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

    E * begin(){
        return pin_ptrs.front();
    }

    E * end(){
        return this->begin() + N;
    }

    void bindPin(Gpio & gpio, const size_t index){
        if(index >= N)return;
        pin_ptrs[index] = &(gpio);
    }

    void writeByIndex(const int index, const bool data) override{
        if(!isIndexValid(index)) return;
        pin_ptrs[index]->write(data);
    }

    bool readByIndex(const int index) override{
        if(!isIndexValid(index))return false;
        return pin_ptrs[index]->read();
    }

    void set(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(pin_ptrs[i]->isValid() and (data & (1 << i))) pin_ptrs[i]->set();
        }
    }
    void clr(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(pin_ptrs[i]->isValid() and data & (1 << i)) pin_ptrs[i]->clr();
        }
    }

    void set(const Pin pin) override{
        const auto i = CTZ((uint16_t)pin);
        if(isIndexValid(i)) pin_ptrs[]->set();
    }
    void clr(const Pin pin) override{
        const auto i = CTZ((uint16_t)pin);
        if(isIndexValid(i)) pin_ptrs[i]->clr();
    }


    bool isIndexValid(const size_t index){return (index < N and pin_ptrs[index]->isValid());}

    E & operator [](const size_t index){return isIndexValid(index) ? *pin_ptrs[index] : GpioNull;}

    void setMode(const int8_t & index, const PinMode & mode) override{
        if(!isIndexValid(index))return;
        pin_ptrs[index]->setMode(mode);
    }
};

#endif