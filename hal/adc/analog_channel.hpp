#pragma once

#include "sys/math/real.hpp"

namespace ymd{

struct AnalogChannel{
public:


};

class AnalogInChannel: public AnalogChannel{
public:
    virtual operator real_t() = 0;
};

class AnalogOutChannel: public AnalogChannel{
protected:
    virtual void write(const uint32_t data) = 0;
public:

    AnalogOutChannel & operator = (const real_t value){
        write(uni_to_u16(value));
        return *this;
    }
};

};