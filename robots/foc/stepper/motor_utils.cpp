#include "motor_utils.hpp"

real_t MetaData::get_max_leadrad(){
    return MIN(ABS(curr) * real_t(1.0), max_leadrad);
}

real_t MetaData::get_max_raddiff(){
    return get_max_leadrad() + real_t(PI / 2);
}