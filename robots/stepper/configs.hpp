#pragma once

#include "constants.hpp"

struct CurrentCtrlConfig{
    real_t current_slew_rate;   //20A/S
    real_t current_clamp;

    constexpr void init(){
        current_slew_rate = 20.0 / foc_freq;
        current_clamp = 0.7;
    }
};


struct GeneralSpeedCtrlConfig{
    real_t max_spd = 40;
    real_t kp = 4;
    real_t kp_clamp = 60;

    real_t kd = 40;
    real_t kd_active_radius = 1.2;
    real_t kd_clamp = 2.4;
};
