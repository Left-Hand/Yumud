#include "current_ctrl.hpp"
#include "types/range/range.hpp"

using namespace ymd;

real_t CurrentController::update(const real_t targ_current, const real_t real_current){
    scexpr Range2_t<real_t> duty_range = {0, real_t(0.95)};

    real_t error = targ_current - real_current;
    real_t kp_contribute = error * kp;

    real_t delta = kp_contribute;

    return duty = duty_range.clamp(duty + delta);
}