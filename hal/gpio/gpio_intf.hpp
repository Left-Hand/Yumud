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

    hal::GpioIntf & outpp(){set_mode(GpioMode::OutPP);return *this;}
    hal::GpioIntf & outod(){set_mode(GpioMode::OutOD);return *this;}
    hal::GpioIntf & afpp(){set_mode(GpioMode::OutAfPP);return *this;}
    hal::GpioIntf & afod(){set_mode(GpioMode::OutAfOD);return *this;}

    hal::GpioIntf & outpp(const BoolLevel initial_state){set_mode(GpioMode::OutPP);write(bool(initial_state));return *this;}
    hal::GpioIntf & outod(const BoolLevel initial_state){set_mode(GpioMode::OutOD);write(bool(initial_state));return *this;}
    hal::GpioIntf & afpp(const BoolLevel initial_state){set_mode(GpioMode::OutAfPP);write(bool(initial_state));return *this;}
    hal::GpioIntf & afod(const BoolLevel initial_state){set_mode(GpioMode::OutAfOD);write(bool(initial_state));return *this;}

    hal::GpioIntf & inana(){set_mode(GpioMode::InAnalog);return *this;}
    hal::GpioIntf & inflt(){set_mode(GpioMode::InFloating);return *this;}
    hal::GpioIntf & inpu(){set_mode(GpioMode::InPullUP);return *this;}
    hal::GpioIntf & inpd(){set_mode(GpioMode::InPullDN);return *this;}

    bool valid() const{return (index() >= 0);}
    virtual int8_t index() const = 0;
    virtual void set_mode(const GpioMode mode) = 0;
    
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