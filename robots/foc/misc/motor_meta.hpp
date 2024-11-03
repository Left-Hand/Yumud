#pragma once

#include "sys/math/real.hpp"
#include "types/vector2/vector2_t.hpp"
#include "types/range/range_t.hpp"

#include "motor_scexpr.hpp"


namespace yumud::foc{
namespace MotorUtils{
    
struct MetaData{
    real_t accel = 0;
    real_t curr = 0;
    real_t spd = 0;
    real_t pos = 0;
    real_t elecrad = 0;
    real_t raddiff = 0;
    real_t radfix = 0;

    real_t openloop_curr;

    Range pos_limit = Range::INF;
    real_t max_curr = real_t(0.7);
    real_t max_spd = 30;
    real_t max_acc = 30;
    
    real_t curr_to_leadrad_ratio = 0;
    real_t spd_to_leadrad_ratio = 0;
    real_t max_leadrad = 0;
    
    real_t targ_curr;
    
    real_t targ_est_spd;
    real_t targ_pos;
    
    real_t targ_spd;
    
    void reset();
    
    __fast_inline constexpr real_t get_max_leadrad();
    __fast_inline constexpr real_t get_max_raddiff();
};

constexpr real_t MetaData::get_max_leadrad(){
    return MIN( ABS(curr) * curr_to_leadrad_ratio, 
                ABS(spd) * spd_to_leadrad_ratio,
                max_leadrad);
}

constexpr real_t MetaData::get_max_raddiff(){
    return get_max_leadrad() + pi_2;
}

}

};