#pragma once


#include "concept/analog_channel.hpp"
#include "concept/pwm_channel.hpp"
#include "drivers/GateDriver/uvw_pwmgen.hpp"

struct PwmIntf;

namespace ymd::drivers{



struct MP6540 final{
    static constexpr auto make_adc_scaler(uint32_t so_res_ohms){
        constexpr q16 curr_mirror_ratio = q16(9200.0);
        const auto volt_to_curr_ratio = curr_mirror_ratio / so_res_ohms;
        return Rescaler<q16>::from_scale(volt_to_curr_ratio);
    }
};

};