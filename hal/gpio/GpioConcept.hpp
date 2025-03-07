#pragma once

#include "gpio_utils.hpp"

namespace ymd::hal{
class GpioIntf{
public:
    // const int8_t pin_index = 0;
public:
    GpioIntf() = default;
    virtual ~GpioIntf() = default;
    GpioIntf(const hal::GpioIntf & other) = delete;
    GpioIntf(hal::GpioIntf && other) = delete;
    
    virtual void set() = 0;
    virtual void clr() = 0;
    virtual void write(const bool val) = 0;
    virtual bool read() const = 0;
    bool toggle() {bool val = !this->read(); write(val); return val;}

    hal::GpioIntf & outpp(){setMode(GpioMode::OutPP);return *this;}
    hal::GpioIntf & outod(){setMode(GpioMode::OutOD);return *this;}
    hal::GpioIntf & afpp(){setMode(GpioMode::OutAfPP);return *this;}
    hal::GpioIntf & afod(){setMode(GpioMode::OutAfOD);return *this;}

    hal::GpioIntf & outpp(const BoolLevel initial_state){setMode(GpioMode::OutPP);write(bool(initial_state));return *this;}
    hal::GpioIntf & outod(const BoolLevel initial_state){setMode(GpioMode::OutOD);write(bool(initial_state));return *this;}
    hal::GpioIntf & afpp(const BoolLevel initial_state){setMode(GpioMode::OutAfPP);write(bool(initial_state));return *this;}
    hal::GpioIntf & afod(const BoolLevel initial_state){setMode(GpioMode::OutAfOD);write(bool(initial_state));return *this;}

    hal::GpioIntf & inana(){setMode(GpioMode::InAnalog);return *this;}
    hal::GpioIntf & inflt(){setMode(GpioMode::InFloating);return *this;}
    hal::GpioIntf & inpu(){setMode(GpioMode::InPullUP);return *this;}
    hal::GpioIntf & inpd(){setMode(GpioMode::InPullDN);return *this;}

    bool valid() const{return (index() >= 0);}
    virtual int8_t index() const = 0;
    virtual void setMode(const GpioMode mode) = 0;
    
    hal::GpioIntf & operator = (const bool val){
        write(val);
        return *this;
    }

    hal::GpioIntf & operator = (const hal::GpioIntf & other){
        write(other.read());
        return *this;
    }

    operator bool() const {return(this->read());}
};

}