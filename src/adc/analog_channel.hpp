#ifndef __ANALOG_CHANNEL_HPP__

#define __ANALOG_CHANNEL_HPP__

#include "adc_channel.hpp"
#include "types/real.hpp"

class AdcChannelOnChip;

class AnalogChannelBase{
protected:
    uint8_t valid_bits = 12;
    real_t uni_to_voltage_scale = real_t(3.3);

    virtual uint32_t getData() = 0;
public:
    real_t getVoltage(){
        real_t uni = real_t(1);
        if(valid_bits <= 16){
            uint16_t data = getData();
            u16_to_uni(data, uni);
        }

        return uni * uni_to_voltage_scale;
    }
};


class AnalogChannel : public AnalogChannelBase{
protected:
    AdcChannelOnChip & channel;
public:
    AnalogChannel(AdcChannelOnChip & _channel):channel(_channel) {;}
};

class AnalogChannelVirtual : public AnalogChannelBase{


};
#endif