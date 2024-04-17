// #ifndef __ADC_CHANNEL_HPP__

// #define __ADC_CHANNEL_HPP__

// #include "src/platform.h"
// #include "analog_channel.hpp"
// #include "adc_enums.h"
// #include "src/gpio/gpio.hpp"
// #include "adc/adc.hpp"


// class AdcOnChip;
// class AdcPrimary;
// class AdcCompanion;

// class AdcChannel{

// };

// class AdcChannelOnChip: public AdcChannel{

// protected:
//     using Channel = AdcChannels;
//     using SampleCycles = AdcSampleCycles;

//     AdcOnChip & instance;
//     Channel channel;
//     // SampleTime sample_time;

//     // virtual void setCaliData(const uint16_t data) = 0;

//     friend class AdcOnChip;
//     friend class AdcPrimary;
//     friend class AdcCompanion;
// public:
//     AdcChannelOnChip(AdcOnChip & _instance, const Channel & _channel):
//             instance(_instance), channel(_channel){};

//     virtual void setSampleTime(const SampleTime time) = 0;

//     void installToPin(const bool en = true);

//     void init(){
//         installToPin();
//     }
// };


// #endif


