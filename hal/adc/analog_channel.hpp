#pragma once

#include "core/math/realmath.hpp"

namespace ymd::hal{

struct AnalogChannel{
public:


};

class AnalogInIntf: public AnalogChannel{
public:
    virtual operator real_t() = 0;

    virtual ~ AnalogInIntf() = default;
};

class AnalogOutIntf: public AnalogChannel{
protected:
    virtual void write(const uint32_t data) = 0;
public:
    virtual ~AnalogOutIntf() = default;

    AnalogOutIntf & operator = (const real_t value){
        write(uni_to_u16(value));
        return *this;
    }
};

};