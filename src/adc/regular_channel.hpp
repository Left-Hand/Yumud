#ifndef __REGULAR_CHANNEL_HPP__

#define __REGULAR_CHANNEL_HPP__

#include "adc_channel.hpp"

class RegularChannel: public AdcChannelOnChip{
public:

protected:

    int16_t cali_data = 0;
    uint16_t temp_data = 0;

    RegularChannel(ADC_TypeDef * _instance = ADC1,const Channel _channel = Channel::CH0, const SampleTime _sample_time = SampleTime::T28_5):
            AdcChannelOnChip(_instance, _channel, _sample_time){;}

    RegularChannel & operator = (const uint16_t & data){temp_data = CLAMP(data + cali_data, 0, 4095); return *this;}
    friend class Adc;
public:
    void setSampleTime(const SampleTime _sample_time) override{
        sample_time = _sample_time;
        uint8_t ch = (uint8_t)channel;
        uint8_t offset = ch % 10;
        offset *= 3;

        if(ch < 10){
            uint32_t tempreg = instance->SAMPTR1;
            tempreg &= ~(0xb111 << offset);
            tempreg |= (uint8_t)sample_time << offset;
            instance->SAMPTR1 = tempreg;
        }else{
            uint32_t tempreg = instance->SAMPTR2;
            tempreg &= ~(0xb111 << offset);
            tempreg |= (uint8_t)sample_time << offset;
            instance->SAMPTR2 = tempreg;
        }
    }

    void setCaliData(const uint16_t _cali_data) override{
        cali_data = _cali_data;
    }

    void installToPin() override;

    operator uint16_t() const {return temp_data;}
};

#endif