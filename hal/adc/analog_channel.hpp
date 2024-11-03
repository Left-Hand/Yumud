#pragma once

#include "sys/math/real.hpp"

namespace yumud{

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
        uint16_t data16;
        uni_to_u16(value, data16);
        write(data16);
        return *this;
    }
};

};