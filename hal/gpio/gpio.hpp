#pragma once

#include "gpio_intf.hpp"
#include "gpio_tag.hpp"


namespace ymd::hal{

class Exti;

class Gpio;

Gpio make_gpio(void * inst, const Nth nth);
Gpio make_gpio(PortSource port_source, const PinSource pin_source);
Gpio make_gpio(PortSource port_source, const Nth nth);

class [[nodiscard]] Gpio final: public GpioIntf{
public:
    ~Gpio(){};

    void set_high();
    void set_low();

    //BSHR的寄存器在BCR前 {1->BSHR; 0->BCR} 使用逻辑操作而非判断以提高速度
    void write(const BoolLevel val);

    BoolLevel read() const;
    __fast_inline void toggle() {
        write(~read());
    }

    hal::Gpio & operator = (const BoolLevel level){
        write(level);
        return *this;
    }

    void set_mode(const GpioMode mode) ;
    __fast_inline void * inst() const {return inst_;} 
    __fast_inline Nth nth() const {
        return Nth(CTZ(static_cast<uint16_t>(pin_nth_)));
    }

    constexpr PinSource pin_nth() const {return pin_nth_;}

    PortSource port() const ;
private:
    void * inst_;
    const PinSource pin_nth_;


    Gpio(void * inst, const PinSource pin_nth);

    friend class VGpio;
    friend class ExtiChannel;
    friend class GpioPort;
    friend Gpio make_gpio(void * inst, const Nth nth);
    friend Gpio make_gpio(PortSource port_source, const PinSource pin_source);
    friend Gpio make_gpio(PortSource port_source, const Nth nth);
};

__inline Gpio make_gpio(void * inst, const Nth nth){
    return Gpio{inst, std::bit_cast<PinSource>(uint16_t(1u << nth.count()))};
}

template<typename PinTag>
static Gpio pintag_to_pin(){
    if constexpr (std::is_same_v<PinTag, void>){
        __builtin_trap();
    }else{
        static constexpr auto _PORT = PinTag::PORT;
        
        static constexpr auto _PIN = PinTag::PIN;
        return make_gpio(_PORT, _PIN);
    }
};
}