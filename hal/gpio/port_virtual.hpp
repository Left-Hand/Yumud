#pragma once

#include "port.hpp"
#include "gpio.hpp"
#include <memory>
#include <array>


namespace ymd::hal{

template<size_t N>
class VGpioPortIntf : public GpioPortIntf{
protected:
    bool isIndexValid(const size_t index){return (index < N);}

    virtual void writePort(const uint16_t data) = 0;
    virtual uint16_t readPort() = 0;
public:


constexpr size_t size(){
    return N;
}

    void setPin(const uint16_t mask) override {
        uint16_t raw = readPort();
        writePort(raw | mask);
    }
    
    void clrPin(const uint16_t mask) override {
        uint16_t raw = readPort();
        writePort(raw & (~mask));
    }

    VGpioPortIntf & operator = (const uint16_t data) override {writePort(data); return *this;}
    operator uint16_t(){return readPort();}
};

template<size_t N>
class VGpioPort final: public VGpioPortIntf<N>{
protected:
    using E = GpioIntf;
    std::array<E *, N> pin_ptrs = {nullptr};

    void writePort(const uint16_t data) override {
        for(uint8_t i = 0; i < 16; i++){
            writeByIndex(i, bool(data & (1 << i)));
        }
    }

    uint16_t readPort() override {
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(pin_ptrs[i]->read() << i);
        }
        return data;
    }
public:
    VGpioPort(){;}

    VGpioPort(const VGpioPort<N> & other){
        pin_ptrs = other.pin_ptrs;
    }

    VGpioPort(VGpioPort<N> && other){
        pin_ptrs = std::move(other.pin_ptrs);
    }

    void bindPin(hal::GpioIntf & gpio, const size_t index){
        if(index >= N)return;
        pin_ptrs[size_t(index)] = &gpio;
    }

    void writeByIndex(const int index, const bool data) override{
        if(!isIndexValid(size_t(index))) return;
        pin_ptrs[size_t(index)]->write(data);
    }

    bool readByIndex(const int index) override{
        if(!isIndexValid(size_t(index)))return false;
        return bool(*(pin_ptrs[size_t(index)]));
    }

    void setPin(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->set();
        }
    }

    void clrPin(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->clr();
        }
    }

    E * begin(){
        return pin_ptrs.front();
    }

    E * end(){
        return begin() + N;
    }

    bool isIndexValid(const size_t index){return (index < N and pin_ptrs[size_t(index)] != nullptr);}

    E & operator [](const size_t index){
        if(isIndexValid(size_t(index))) 
            return *pin_ptrs[size_t(index)];
        else
            return NullGpio;
    }

    void setMode(const int index, const GpioMode mode) override{
        if(index < 0) return;
        if(!isIndexValid(size_t(index)))return;
        pin_ptrs[size_t(index)]->setMode(mode);
    }
};

template<size_t N>
class VGpioPortLocal final: public VGpioPortIntf<N>{
protected:
    using E = Gpio;
    std::array<E *, N> pin_ptrs = {nullptr};

    void write(const uint16_t data) override {
        for(size_t i = 0; i < N; i++){
            writeByIndex(i, bool(data & (1 << i)));
        }
    }
    uint16_t read() override {
        uint16_t data = 0;
        for(size_t i = 0; i < N; i++){
            data |= uint16_t(pin_ptrs[i]->read() << i);
        }
        return data;
    }
public:
    VGpioPortLocal(){;}
    void init(){;}

    E * begin(){
        return pin_ptrs.front();
    }

    E * end(){
        return this->begin() + N;
    }

    void bindPin(Gpio & gpio, const size_t index){
        if(index >= N)return;
        pin_ptrs[size_t(index)] = &(gpio);
    }

    void writeByIndex(const int index, const bool data) override{
        if(!isIndexValid(size_t(index))) return;
        pin_ptrs[size_t(index)]->write(data);
    }

    bool readByIndex(const int index) override{
        if(!isIndexValid(size_t(index)))return false;
        return pin_ptrs[size_t(index)]->read();
    }

    void setPin(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(pin_ptrs[i]->isValid() and (data & (1 << i))) pin_ptrs[i]->setPin();
        }
    }
    void clrPin(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(pin_ptrs[i]->isValid() and data & (1 << i)) pin_ptrs[i]->clrPin();
        }
    }

    void setPin(const Pin pin) override{
        const auto i = CTZ(uint16_t(pin));
        if(isIndexValid(i)) pin_ptrs[]->setPin();
    }
    void clrPin(const Pin pin) override{
        const auto i = CTZ(uint16_t(pin));
        if(isIndexValid(i)) pin_ptrs[i]->clrPin();
    }


    bool isIndexValid(const size_t index){return (index < N and pin_ptrs[size_t(index)]->isValid());}

    E & operator [](const size_t index){return isIndexValid(size_t(index)) ? *pin_ptrs[size_t(index)] : NullGpio;}

    void setMode(const int8_t & index, const GpioMode & mode) override{
        if(!isIndexValid(size_t(index)))return;
        pin_ptrs[size_t(index)]->setMode(mode);
    }
};


}