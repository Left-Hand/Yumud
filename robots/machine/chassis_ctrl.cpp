#include "chassis_module.hpp"

using namespace ymd;
using namespace ymd::robots;

static __inline real_t signed_sqrt(const real_t x){
    return SIGN_AS(sqrt(ABS(x)), x);
}

static __inline Vec2<real_t> signed_sqrt(const Vec2<real_t> vec){
    return {signed_sqrt(vec.x), signed_sqrt(vec.y)};
}
