#pragma once

#include "core/math/real.hpp"

namespace ymd::hal{


class AnalogInIntf{
public:
    virtual operator real_t() = 0;
    real_t get_voltage(){return real_t(*this);}
    virtual ~ AnalogInIntf() = default;
};

class AnalogOutIntf{
protected:
    virtual void write(const uint32_t data) = 0;
public:
    virtual ~AnalogOutIntf() = default;

    AnalogOutIntf & operator = (const real_t value){
        write(uni_to_u16(value));
        return *this;
    }

    void set_voltage(const real_t volt){
        *this = volt;
    }
};

};