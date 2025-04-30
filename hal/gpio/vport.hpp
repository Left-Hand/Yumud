#pragma once

#include "gpio_port_intf.hpp"
#include "gpio_intf.hpp"

#include <array>


namespace ymd::hal{

template<size_t N>
class VGpioPortIntf : public GpioPortIntf{
protected:
    bool is_index_valid(const size_t index){return (index < N);}
public:
    constexpr size_t size(){
        return N;
    }

    void set_by_mask(const uint16_t mask) override {
        uint16_t raw = read_mask();
        write_by_mask(raw | mask);
    }
    
    void clr_by_mask(const uint16_t mask) override {
        uint16_t raw = read_mask();
        write_by_mask(raw & (~mask));
    }

};

template<size_t N>
class VGpioPort final: public VGpioPortIntf<N>{
protected:
    using E = hal::GpioIntf;
    std::array<E *, N> pin_ptrs = {nullptr};

    void write_by_mask(const uint16_t data) override {
        for(uint8_t i = 0; i < 16; i++){
            write_by_index(i, BoolLevel::from(bool(data & (1 << i))));
        }
    }

    uint16_t read_mask() override {
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(bool(pin_ptrs[i]->read()) << i);
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

    void bind_pin(hal::GpioIntf & gpio, const size_t index){
        if(index >= N) while(true);
        pin_ptrs[size_t(index)] = &gpio;
    }

    void write_by_index(const size_t index, const BoolLevel data) override{
        if(!is_index_valid(size_t(index))) return;
        pin_ptrs[size_t(index)]->write((data));
    }

    BoolLevel read_by_index(const size_t index) override{
        if(!is_index_valid(size_t(index)))return LOW;
        return (pin_ptrs[size_t(index)])->read();
    }

    void set_by_mask(const uint16_t data) override{
        for(uint8_t i = 0; i < 16; i++){
            if(data & (1 << i)) pin_ptrs[i]->set();
        }
    }

    void clr_by_mask(const uint16_t data) override{
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

    bool is_index_valid(const size_t index){
        return (likely(index < N) and likely(pin_ptrs[size_t(index)]));
    }

    bool is_index_empty(const size_t index){
        return (likely(index < N) and likely(pin_ptrs[size_t(index)] == nullptr));
    }

    E & operator [](const size_t index){
        if(is_index_valid(size_t(index))) return *pin_ptrs[size_t(index)];
        while(true);
    }

    void set_mode(const size_t index, const GpioMode mode) override{
        if(index < 0) return;
        if(!is_index_valid(size_t(index)))return;
        pin_ptrs[size_t(index)]->set_mode(mode);
    }
};

// template<size_t N>
// class VGpioPortLocal final: public VGpioPortIntf<N>{
// protected:
//     using E = Gpio;
//     std::array<E *, N> pin_ptrs = {nullptr};

//     void write(const uint16_t data) override {
//         for(size_t i = 0; i < N; i++){
//             write_by_index(i, bool(data & (1 << i)));
//         }
//     }
//     uint16_t read() override {
//         uint16_t data = 0;
//         for(size_t i = 0; i < N; i++){
//             data |= uint16_t(pin_ptrs[i]->read() << i);
//         }
//         return data;
//     }
// public:
//     VGpioPortLocal(){;}
//     void init(){;}

//     E * begin(){
//         return pin_ptrs.front();
//     }

//     E * end(){
//         return this->begin() + N;
//     }

//     void bindPin(Gpio & gpio, const size_t index){
//         if(index >= N)return;
//         pin_ptrs[size_t(index)] = &(gpio);
//     }

//     void write_by_index(const size_t index, const bool data) override{
//         if(!isIndexValid(size_t(index))) return;
//         pin_ptrs[size_t(index)]->write(data);
//     }

//     bool read_by_index(const size_t index) override{
//         if(!isIndexValid(size_t(index)))return false;
//         return pin_ptrs[size_t(index)]->read();
//     }

//     void set_pin(const uint16_t data) override{
//         for(uint8_t i = 0; i < 16; i++){
//             if(pin_ptrs[i]->isValid() and (data & (1 << i))) pin_ptrs[i]->set_pin();
//         }
//     }
//     void clr_pin(const uint16_t data) override{
//         for(uint8_t i = 0; i < 16; i++){
//             if(pin_ptrs[i]->isValid() and data & (1 << i)) pin_ptrs[i]->clr_pin();
//         }
//     }

//     void set_pin(const Pin pin) override{
//         const auto i = CTZ(uint16_t(pin));
//         if(isIndexValid(i)) pin_ptrs[]->set_pin();
//     }
//     void clr_pin(const Pin pin) override{
//         const auto i = CTZ(uint16_t(pin));
//         if(isIndexValid(i)) pin_ptrs[i]->clr_pin();
//     }


//     bool isIndexValid(const size_t index){return (index < N and pin_ptrs[size_t(index)]->isValid());}

//     E & operator [](const size_t index){return isIndexValid(size_t(index)) ? *pin_ptrs[size_t(index)] : NullGpio;}

//     void set_mode(const int8_t & index, const GpioMode & mode) override{
//         if(!isIndexValid(size_t(index)))return;
//         pin_ptrs[size_t(index)]->set_mode(mode);
//     }
// };


}