#pragma once

#include "gpio_intf.hpp"
#include "gpio_port_intf.hpp"

namespace ymd::hal{
class Gpio;


class VGpio final :public GpioIntf{
protected:
    GpioPortIntf & port_;
    Nth nth_;
public:
    VGpio(const VGpio &) = delete;
    VGpio(VGpio &&) = delete;
    VGpio(GpioPortIntf & port, const PinNth pin_nth):
        port_(port), nth_(Nth(CTZ(uint16_t(pin_nth)))){;}

    __fast_inline void set() {port_.set_nth(nth_);}
    __fast_inline void clr() {port_.clr_nth(nth_);}
    __fast_inline void write(const BoolLevel val){port_.write_nth(nth_, val);}
    __fast_inline BoolLevel read() const {return port_.read_nth(nth_);}
    __fast_inline Nth nth() const {return nth_;}
    void set_mode(const GpioMode mode) { port_.set_mode(nth_, mode);}
};


}