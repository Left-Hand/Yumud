#include "current_ctrl.hpp"
#include "types/regions/range2/range2.hpp"

using namespace ymd;

real_t CurrentController::update(const real_t targ_current, const real_t real_current){
    scexpr Range2<real_t> duty_range = {0_r, 0.95_r};

    real_t error = targ_current - real_current;
    real_t kp_contribute = error * kp;

    real_t delta = kp_contribute;

    return duty = duty_range.clamp(duty + delta);
}