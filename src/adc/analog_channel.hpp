#ifndef __ANALOG_CHANNEL_HPP__

#define __ANALOG_CHANNEL_HPP__

#include "adc_channel.hpp"

class AnalogChannelBase{

};

class AnalogChannel : public AnalogChannelBase{
protected:
    AdcChannelOnChip & channel;
public:
    AnalogChannel(AdcChannelOnChip & _channel):channel(_channel) {;}
}


#endif