#pragma once

#include "gpio_utils.hpp"

namespace ymd::hal{
class GpioConcept{
public:
    const int8_t pin_index = 0;
public:
    GpioConcept(int8_t _pin_index):pin_index(_pin_index){;}
    GpioConcept(const GpioConcept & other) = delete;
    GpioConcept(GpioConcept && other) = delete;
    
    virtual void set() = 0;
    virtual void clr() = 0;
    virtual void write(const bool val) = 0;
    virtual bool read() const = 0;
    bool toggle() {bool val = !this->read(); write(val); return val;}

    GpioConcept & outpp(){setMode(GpioMode::OutPP);return *this;}
    GpioConcept & outod(){setMode(GpioMode::OutOD);return *this;}
    GpioConcept & afpp(){setMode(GpioMode::OutAfPP);return *this;}
    GpioConcept & afod(){setMode(GpioMode::OutAfOD);return *this;}

    GpioConcept & outpp(const BoolLevel initial_state){setMode(GpioMode::OutPP);write(bool(initial_state));return *this;}
    GpioConcept & outod(const BoolLevel initial_state){setMode(GpioMode::OutOD);write(bool(initial_state));return *this;}
    GpioConcept & afpp(const BoolLevel initial_state){setMode(GpioMode::OutAfPP);write(bool(initial_state));return *this;}
    GpioConcept & afod(const BoolLevel initial_state){setMode(GpioMode::OutAfOD);write(bool(initial_state));return *this;}

    GpioConcept & inana(){setMode(GpioMode::InAnalog);return *this;}
    GpioConcept & inflt(){setMode(GpioMode::InFloating);return *this;}
    GpioConcept & inpu(){setMode(GpioMode::InPullUP);return *this;}
    GpioConcept & inpd(){setMode(GpioMode::InPullDN);return *this;}

    bool isValid() const {return pin_index >= 0;}
    int8_t getIndex() const {return pin_index;}
    virtual void setMode(const GpioMode mode) = 0;
    
    GpioConcept & operator = (const bool _val){
        write(_val);
        return *this;
    }

    GpioConcept & operator = (const GpioConcept & other){
        write(other.read());
        return *this;
    }

    operator bool() const {return(this->read());}
};

}