#pragma once

#include "gpio_port_intf.hpp"
#include "gpio_intf.hpp"

#include <array>


namespace ymd::hal{

template<size_t N>
class VGpioPortIntf : public GpioPortIntf{
// protected:
//     bool is_index_valid(const size_t index){return (index < N);}
public:
    constexpr size_t size(){
        return N;
    }

    void set_by_mask(const PinMask mask)  {
        const auto raw = read_mask();
        write_by_mask(raw | mask);
    }
    
    void clr_by_mask(const PinMask mask)  {
        const auto raw = read_mask();
        write_by_mask(raw & (~mask));
    }

};

template<size_t N>
class VGpioPort final: public VGpioPortIntf<N>{
protected:
    using E = hal::GpioIntf;
    std::array<E *, N> p_pins_ = {nullptr};

    void write_by_mask(const PinMask mask)  {
        for(uint8_t i = 0; i < 16; i++){
            write_nth(i, BoolLevel::from(mask.test(i)));
        }
    }

    PinMask read_mask()  {
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(p_pins_[i]->read().to_bool() << i);
        }
        return PinMask(data);
    }
public:
    __fast_inline VGpioPort(){;}

    __fast_inline VGpioPort(const VGpioPort<N> & other){
        p_pins_ = other.p_pins_;
    }

    __fast_inline VGpioPort(VGpioPort<N> && other){
        p_pins_ = std::move(other.p_pins_);
    }

    __fast_inline void bind_pin(hal::GpioIntf & gpio, const size_t index){
        if(index >= N) while(true);
        p_pins_[size_t(index)] = &gpio;
    }

    __fast_inline void write_nth(const size_t index, const BoolLevel data) {
        if(!is_index_valid(size_t(index))) return;
        p_pins_[size_t(index)]->write((data));
    }

    __fast_inline BoolLevel read_nth(const size_t index) {
        if(!is_index_valid(size_t(index)))return LOW;
        return (p_pins_[size_t(index)])->read();
    }

    __fast_inline void set_by_mask(const PinMask mask) {
        for(uint8_t i = 0; i < 16; i++){
            if(mask.test(i)) p_pins_[i]->set();
        }
    }

    __fast_inline void clr_by_mask(const PinMask mask) {
        for(uint8_t i = 0; i < 16; i++){
            if(mask.test(i)) p_pins_[i]->clr();
        }
    }

    __fast_inline E * begin(){
        return p_pins_.front();
    }

    __fast_inline E * end(){
        return begin() + N;
    }

    __fast_inline bool is_index_valid(const size_t index){
        return (likely(index < N) and likely(p_pins_[size_t(index)]));
    }

    __fast_inline bool is_index_empty(const size_t index){
        return (likely(index < N) and likely(p_pins_[size_t(index)] == nullptr));
    }

    __fast_inline E & operator [](const size_t index){
        if(is_index_valid(size_t(index))) return *p_pins_[size_t(index)];
        while(true);
    }

    __fast_inline  void set_mode(const size_t index, const GpioMode mode) {
        if(index < 0) return;
        if(!is_index_valid(size_t(index)))return;
        p_pins_[size_t(index)]->set_mode(mode);
    }
};

// template<size_t N>
// class VGpioPortLocal final: public VGpioPortIntf<N>{
// protected:
//     using E = Gpio;
//     std::array<E *, N> p_pins_ = {nullptr};

//     void write(const uint16_t data)  {
//         for(size_t i = 0; i < N; i++){
//             write_nth(i, bool(data & (1 << i)));
//         }
//     }
//     uint16_t read()  {
//         uint16_t data = 0;
//         for(size_t i = 0; i < N; i++){
//             data |= uint16_t(p_pins_[i]->read() << i);
//         }
//         return data;
//     }
// public:
//     VGpioPortLocal(){;}
//     void init(){;}

//     E * begin(){
//         return p_pins_.front();
//     }

//     E * end(){
//         return this->begin() + N;
//     }

//     void bindPin(Gpio & gpio, const size_t index){
//         if(index >= N)return;
//         p_pins_[size_t(index)] = &(gpio);
//     }

//     void write_nth(const size_t index, const bool data) {
//         if(!isIndexValid(size_t(index))) return;
//         p_pins_[size_t(index)]->write(data);
//     }

//     bool read_nth(const size_t index) {
//         if(!isIndexValid(size_t(index)))return false;
//         return p_pins_[size_t(index)]->read();
//     }

//     void set_pin(const uint16_t data) {
//         for(uint8_t i = 0; i < 16; i++){
//             if(p_pins_[i]->isValid() and (data & (1 << i))) p_pins_[i]->set_pin();
//         }
//     }
//     void clr_pin(const uint16_t data) {
//         for(uint8_t i = 0; i < 16; i++){
//             if(p_pins_[i]->isValid() and data & (1 << i)) p_pins_[i]->clr_pin();
//         }
//     }

//     void set_pin(const PinNth pin) {
//         const auto i = CTZ(uint16_t(pin));
//         if(isIndexValid(i)) p_pins_[]->set_pin();
//     }
//     void clr_pin(const PinNth pin) {
//         const auto i = CTZ(uint16_t(pin));
//         if(isIndexValid(i)) p_pins_[i]->clr_pin();
//     }


//     bool isIndexValid(const size_t index){return (index < N and p_pins_[size_t(index)]->isValid());}

//     E & operator [](const size_t index){return isIndexValid(size_t(index)) ? *p_pins_[size_t(index)] : NullGpio;}

//     void set_mode(const int8_t & index, const GpioMode & mode) {
//         if(!isIndexValid(size_t(index)))return;
//         p_pins_[size_t(index)]->set_mode(mode);
//     }
// };


}