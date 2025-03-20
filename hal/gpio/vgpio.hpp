#pragma once

#include "gpio_intf.hpp"
#include "port_intf.hpp"

namespace ymd::hal{
class Gpio;


class VGpio final :public GpioIntf{
protected:
    GpioPortIntf & port_;
    int8_t pin_index_;
    static GpioPortIntf & form_gpiotypedef_to_port(uint32_t base);
public:
    VGpio(const VGpio &) = delete;
    VGpio(VGpio &&) = delete;

    using GpioIntf::operator=;

    VGpio(const Gpio & gpio);
    VGpio(GpioPortIntf & port, const int8_t pin_index);
    VGpio(GpioPortIntf & port, const Pin pin);
    __fast_inline void set() {port_.setByIndex(pin_index_);}
    __fast_inline void clr() {port_.clrByIndex(pin_index_);}
    __fast_inline void write(const bool val){port_.writeByIndex(pin_index_, val);}
    __fast_inline bool read() const {return port_.readByIndex(pin_index_);}
    __fast_inline int8_t index() const {return pin_index_;}
    void setMode(const GpioMode mode) { port_.setMode(pin_index_, mode);}
};


}