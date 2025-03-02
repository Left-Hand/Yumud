#pragma once

#include "TempSensor.hpp"
#include "hal/adc/adc_channel.hpp"

namespace ymd::drivers{

class NTC:public TempSensor{
protected:
    scexpr real_t R_kOhms = 100;
    scexpr real_t R0_kOhms = 10;

    uint8_t index;

    real_t last_temp = 0;

    hal::AdcChannelConcept & channel_;
    uint B_ = 3950;
    
    real_t get_uniV(){
        return real_t(channel_);
    }

public:

    NTC(hal::AdcChannelConcept & channel, const uint B = 3950):channel_(channel), B_(B){;}

    void init() override{

    };

    void  update() override;



    real_t getRes(){
        real_t VR = get_uniV();
        return VR/(1-VR) * R_kOhms;
    }

    real_t getTemp(){
        return last_temp;
    }
};

};