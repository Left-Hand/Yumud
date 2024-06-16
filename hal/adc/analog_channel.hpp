#pragma once

#include "adc_channel.hpp"
#include "types/real.hpp"

// class AdcChannelOnChip{

// }

// template<int valid_bits>

struct AnalogChannelBasics{
protected:
    uint32_t data;
public:
    real_t scale;
    real_t basis;
};
class AnalogChannelReadable: virtual public AnalogChannelBasics{
protected:
    virtual void read(uint32_t & _data) const = 0;
public:

    operator real_t() const {
        real_t uni = real_t(1);
        uint32_t _data; 
        read(_data);
        u16_to_uni(data, uni);
        return uni;
    }
};

class AnalogChannelWritable: virtual public AnalogChannelBasics{
protected:
    virtual void write(const uint32_t data) = 0;
public:

    AnalogChannelWritable & operator = (const real_t value){
        uint16_t data16;
        uni_to_u16(value, data16);
        write(data16);
        return *this;
    }
};

// class AnalogChannel : public AnalogChannelReadable, public AnalogChannelWritable{
// protected:
// public:
//     AnalogChannel(AdcChannel & _channel):channel(_channel) {;}
// };

// #endif