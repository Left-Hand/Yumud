#include "motor_utils.hpp"

real_t MetaData::get_max_leadrad(){
    return MIN( ABS(curr) * curr_to_leadrad_ratio, 
                ABS(spd) * spd_to_leadrad_ratio,
                max_leadrad);
}

real_t MetaData::get_max_raddiff(){
    return get_max_leadrad() + real_t(PI / 2);
}