#pragma once

#include "adc_channel.hpp"
#include "types/real.hpp"

// class AdcChannelOnChip{

// }

// template<int valid_bits>

struct AnalogChannel{
// protected:
    // uint32_t data;
public:
    real_t scale = 1;
    real_t basis = 0;
};
class AnalogInChannel: virtual public AnalogChannel{
protected:
public:
    virtual real_t uni() const = 0;

    operator real_t() const {
        return uni() * scale + basis;
    }
};

class AnalogOutChannel: virtual public AnalogChannel{
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

// class AnalogChannel : public AnalogInChannel, public AnalogOutChannel{
// protected:
// public:
//     AnalogChannel(AdcChannel & _channel):channel(_channel) {;}
// };

// #endif