#ifndef __ADC_CHANNEL_HPP__

#define __ADC_CHANNEL_HPP__

#include "sys/platform.h"
#include "adc_enums.h"

#include "analog_channel.hpp"
#include "hal/gpio/gpio.hpp"
#include "adc/adc.hpp"


class AdcOnChip;
class AdcPrimary;
class AdcCompanion;


class AdcChannelConcept:public AnalogChannelReadable{

};

class AdcChannelOnChip: public AdcChannelConcept{
protected:
    using Channel = AdcChannel;
    using SampleCycles = AdcSampleCycles;

    ADC_TypeDef * instance;
    Channel channel;

    friend class AdcOnChip;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:
    AdcChannelOnChip(ADC_TypeDef * _instance, const Channel & _channel):
            instance(_instance), channel(_channel){};
};


#endif


