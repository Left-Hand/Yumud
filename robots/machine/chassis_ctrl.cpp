#include "chassis_module.hpp"

using namespace ymd;
using namespace ymd::robots;

static __inline real_t signed_sqrt(const real_t x){
    return SIGN_AS(sqrt(ABS(x)), x);
}

static __inline Vector2<real_t> signed_sqrt(const Vector2<real_t> vec){
    return {signed_sqrt(vec.x), signed_sqrt(vec.y)};
}


real_t RotationCtrl::update(const real_t targ_rad, const real_t rad, const real_t gyr){
    auto rad_err = targ_rad - rad;
    auto gyr_err = 0 - gyr;
    // Ray && curr = {Vector2<real_t>{0,0}, 
    return signed_sqrt(rad_err) * config_.kp +
        gyr_err * config_.kd;
    // };

    // chassis_.setCurrent(curr);
}

Vector2<real_t> PositionCtrl::update(const Vector2<real_t> & targ_pos, const Vector2<real_t> & pos, const Vector2<real_t> & spd){
    auto pos_err = targ_pos - pos;
    auto spd_err = Vector2<real_t>(0,0) - spd;
    return {
        signed_sqrt(pos_err) * config_.kp +
        spd_err * config_.kd};

}