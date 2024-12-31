#pragma once

#include "gpio_utils.hpp"

namespace ymd{
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

    void outpp(){setMode(GpioMode::OutPP);}
    void outod(){setMode(GpioMode::OutOD);}
    void afpp(){setMode(GpioMode::OutAfPP);}
    void afod(){setMode(GpioMode::OutAfOD);}

    void outpp(const bool initial_state){setMode(GpioMode::OutPP);write(initial_state);}
    void outod(const bool initial_state){setMode(GpioMode::OutOD);write(initial_state);}
    void afpp(const bool initial_state){setMode(GpioMode::OutAfPP);write(initial_state);}
    void afod(const bool initial_state){setMode(GpioMode::OutAfOD);write(initial_state);}

    void inana(){setMode(GpioMode::InAnalog);}
    void inflt(){setMode(GpioMode::InFloating);}
    void inpu(){setMode(GpioMode::InPullUP);}
    void inpd(){setMode(GpioMode::InPullDN);}

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