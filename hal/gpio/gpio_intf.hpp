#pragma once

#include "gpio_utils.hpp"

namespace ymd::hal{
class [[nodiscard]] GpioIntf{
public:

    GpioIntf() = default;
    virtual ~GpioIntf() = default;

    virtual void set_high() = 0;
    virtual void set_low() = 0;
    virtual void write(const BoolLevel val) = 0;
    virtual BoolLevel read() const = 0;
    [[nodiscard]] bool is_low() const{return read() == BoolLevel::LOW;}
    [[nodiscard]] bool is_high() const{return read() == BoolLevel::HIGH;}
    virtual Nth nth() const = 0;
    virtual void set_mode(const GpioMode mode) = 0;

    void outpp(){set_mode(GpioMode::OutPP);}
    void outod(){set_mode(GpioMode::OutOD);}
    void afpp(){set_mode(GpioMode::OutAfPP);}
    void afod(){set_mode(GpioMode::OutAfOD);}

    void outpp(const BoolLevel initial_state){
        set_mode(GpioMode::OutPP);
        write((initial_state));
    }

    void outod(const BoolLevel initial_state){
        set_mode(GpioMode::OutOD);
        write((initial_state));
    }

    void afpp(const BoolLevel initial_state){
        set_mode(GpioMode::OutAfPP);
        write((initial_state));
    }

    void afod(const BoolLevel initial_state){
        set_mode(GpioMode::OutAfOD);
        write((initial_state));
    }


    void inana(){set_mode(GpioMode::InAnalog);}
    void inflt(){set_mode(GpioMode::InFloating);}
    void inpu(){set_mode(GpioMode::InPullUP);}
    void inpd(){set_mode(GpioMode::InPullDN);}



    operator BoolLevel() const {return(this->read());}
};

}