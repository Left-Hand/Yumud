#ifndef __INJECTED_CHANNEL_HPP__

#define __INJECTED_CHANNEL_HPP__

#include "adc_channel.hpp"


class InjectedChannel: public AdcChannelOnChip{
protected:
    uint8_t rank;
    uint8_t ch_code;
    InjectedChannel(ADC_TypeDef * _instance = ADC1, const Channel _channel = Channel::CH0, const SampleTime _sample_time = SampleTime::T28_5):
            AdcChannelOnChip(_instance, _channel, _sample_time){
        switch(rank){
        default:
        case 0:
            ch_code = ADC_InjectedChannel_1;
            break;
        case 1:
            ch_code = ADC_InjectedChannel_2;
            break;
        case 2:
            ch_code = ADC_InjectedChannel_3;
            break;
        case 3:
            ch_code = ADC_InjectedChannel_4;
            break;
        }
    }

    void setCaliData(const uint16_t _cali_data) override{
        ADC_SetInjectedOffset(instance, ch_code, _cali_data);
    }

    friend class AdcHw;
    friend class AdcPrimary;
    friend class AdcCompanion;
public:


    void setSampleTime(const SampleTime _sample_time) override{
        ADC_InjectedChannelConfig(instance, ch_code, rank, (uint8_t)_sample_time);
    }

    operator uint16_t() const{
        return ADC_GetInjectedConversionValue(instance, ch_code);
    }

};

#endif