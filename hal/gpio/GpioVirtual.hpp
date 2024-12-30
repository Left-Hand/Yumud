#pragma once

#include "GpioConcept.hpp"
#include "port_concept.hpp"

namespace ymd{
class Gpio;


class GpioVirtual:public GpioConcept{
protected:
    PortConcept & _port;

    static PortConcept & form_gpiotypedef_to_port(uint32_t base);
public:
    DELETE_COPY_AND_MOVE(GpioVirtual)
    using GpioConcept::operator=;

    GpioVirtual(const Gpio & gpio);
    GpioVirtual(PortConcept & port, const int8_t _pin_index);
    GpioVirtual(PortConcept & port, const Pin _pin);
    __fast_inline void set() override {_port.setByIndex(pin_index);}
    __fast_inline void clr() override {_port.clrByIndex(pin_index);}
    __fast_inline void write(const bool val){_port.writeByIndex(pin_index, val);}
    __fast_inline bool read() const override {return _port.readByIndex(pin_index);}

    void setMode(const GpioMode mode) override{ _port.setMode(pin_index, mode);}
};


}