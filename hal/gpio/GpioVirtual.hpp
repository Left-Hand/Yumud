#pragma once

#include "GpioConcept.hpp"
#include "port_concept.hpp"

namespace ymd::hal{
class Gpio;


class VGpio:public GpioConcept{
protected:
    GpioPortConcept & _port;

    static GpioPortConcept & form_gpiotypedef_to_port(uint32_t base);
public:
    DELETE_COPY_AND_MOVE(VGpio)
    using GpioConcept::operator=;

    VGpio(const Gpio & gpio);
    VGpio(GpioPortConcept & port, const int8_t _pin_index);
    VGpio(GpioPortConcept & port, const Pin _pin);
    __fast_inline void set() override {_port.setByIndex(pin_index);}
    __fast_inline void clr() override {_port.clrByIndex(pin_index);}
    __fast_inline void write(const bool val){_port.writeByIndex(pin_index, val);}
    __fast_inline bool read() const override {return _port.readByIndex(pin_index);}

    void setMode(const GpioMode mode) override{ _port.setMode(pin_index, mode);}
};


}