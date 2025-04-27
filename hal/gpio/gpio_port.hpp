#pragma once

#include "gpio.hpp"
#include "gpio_port_intf.hpp"

namespace ymd::hal{
class GpioPort final: public GpioPortIntf{
protected:
    GPIO_TypeDef * instance;
    Gpio channels[16];

    friend class Gpio;
public:
    GpioPort(GPIO_TypeDef * _instance):
        instance(_instance),
        channels{
            Gpio(instance, Pin::_0),
            Gpio(instance, Pin::_1),
            Gpio(instance, Pin::_2),
            Gpio(instance, Pin::_3),
            Gpio(instance, Pin::_4),
            Gpio(instance, Pin::_5),
            Gpio(instance, Pin::_6),
            Gpio(instance, Pin::_7),
            Gpio(instance, Pin::_8),
            Gpio(instance, Pin::_9),
            Gpio(instance, Pin::_10),
            Gpio(instance, Pin::_11),
            Gpio(instance, Pin::_12),
            Gpio(instance, Pin::_13),
            Gpio(instance, Pin::_14),
            Gpio(instance, Pin::_15),
        }{;}
    void init();
    void enableRcc(const bool en = true);


    __inline void write_by_index(const size_t index, const BoolLevel data) override;
    __inline void set_by_mask(const uint16_t data) override;
    __inline void clr_by_mask(const uint16_t data) override;
    __inline void write_by_mask(const uint16_t data){
        instance->OUTDR = data;}

    __inline uint16_t read_mask(){return instance->INDR;}

    Gpio & operator [](const size_t index){
        return channels[index & 0b1111];
    };

    Gpio & operator [](const Pin pin){
        if(pin != Pin::None) return channels[CTZ(uint16_t(pin)) & 0b1111];
        else return Gpio::null();
    };

    void set_mode(const size_t index, const GpioMode mode) override;
};

__inline void GpioPort::write_by_index(const size_t index, const BoolLevel data){
    uint16_t mask = 1 << bool(index);
    if(data){
        set_by_mask(mask);
    }else{
        clr_by_mask(mask);
    }
}

__inline void GpioPort::set_by_mask(const uint16_t data){
    instance->BSHR = data;
}

__inline void GpioPort::clr_by_mask(const uint16_t data){
    instance->BCR = data;
}



#ifdef ENABLE_GPIOA
extern GpioPort portA;
#endif

#ifdef ENABLE_GPIOB
extern GpioPort portB;
#endif

#ifdef ENABLE_GPIOC
extern GpioPort portC;
#endif

#ifdef ENABLE_GPIOD
extern GpioPort portD;
#endif

#ifdef ENABLE_GPIOE
extern GpioPort portE;
#endif

}
