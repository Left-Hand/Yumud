// #ifndef __REGULAR_CHANNEL_HPP__

// #define __REGULAR_CHANNEL_HPP__

// #include "adc_channel.hpp"

// class RegularChannel: public AdcChannelOnChip{
// public:
// protected:
//     RegularChannel(AdcOnChip &  _instance,const Channel &  _channel):
//         AdcChannelOnChip(_instance, _channel){;}

//     // RegularChannel & operator = (const uint16_t & data){temp_data = CLAMP(data + cali_data, 0, 4095); return *this;}


//     friend class AdcOnChip;
//     friend class AdcPrimary;
//     friend class AdcCompanion;

// public:
//     void setSampleTime(const SampleTime _sample_time) override{

//     }

//     operator uint16_t() const {return temp_data;}
// };

// #endif