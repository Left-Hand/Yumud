#pragma once

#include "gpio_port_intf.hpp"
#include "gpio_intf.hpp"

#include <array>


namespace ymd::hal{

template<size_t N>
class VGpioPortIntf : public GpioPortIntf{
// protected:
//     bool is_nth_valid(const Nth nth){return (index < N);}
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
            const auto nth = Nth(i);
            write_nth(nth, BoolLevel::from(mask.test(nth)));
        }
    }

    PinMask read_mask()  {
        uint16_t data = 0;
        for(uint8_t i = 0; i < 16; i++){
            data |= uint16_t(p_pins_[i]->read().to_bool() << i);
        }
        return PinMask::from_u16(data);
    }
public:
    __fast_inline VGpioPort(){
        p_pins_.fill(nullptr);
    }
    __fast_inline void bind_pin(hal::GpioIntf & gpio, const Nth nth){
        if(!is_nth_valid(nth)) return;
        p_pins_[nth.count()] = &gpio;
    }

    __fast_inline void write_nth(const Nth nth, const BoolLevel data) {
        if(!is_nth_valid(nth)) return;
        p_pins_[nth.count()]->write((data));
    }

    __fast_inline BoolLevel read_nth(const Nth nth) {
        if(!is_nth_valid(nth))return LOW;
        return (p_pins_[nth.count()])->read();
    }

    __fast_inline void set_by_mask(const PinMask mask) {
        for(uint8_t i = 0; i < 16; i++){
            if(mask.test(Nth(i))) p_pins_[i]->set();
        }
    }

    __fast_inline void clr_by_mask(const PinMask mask) {
        for(uint8_t i = 0; i < 16; i++){
            if(mask.test(Nth(i))) p_pins_[i]->clr();
        }
    }

    [[nodiscard]] __fast_inline E * begin(){
        return p_pins_.front();
    }

    [[nodiscard]] __fast_inline E * end(){
        return begin() + N;
    }

    [[nodiscard]] __fast_inline bool is_nth_valid(const Nth nth){
        return (likely(nth.count() < N));
    }

    [[nodiscard]] __fast_inline bool is_nth_available(const Nth nth){
        return is_nth_valid(nth) and (p_pins_[nth.count()] == nullptr);
    }

    [[nodiscard]] __fast_inline E & operator [](const Nth nth){
        if(is_nth_valid(nth)) return *p_pins_[nth.count()];
        while(true);
    }

    __fast_inline  void set_mode(const Nth nth, const GpioMode mode) {
        if(!is_nth_valid(nth))return;
        p_pins_[nth.count()]->set_mode(mode);
    }
};

}