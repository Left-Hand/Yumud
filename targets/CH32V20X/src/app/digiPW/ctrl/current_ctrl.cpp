#include "current_ctrl.hpp"
#include "types/range/range_t.hpp"

real_t CurrentController::update(const real_t targ_current, const real_t real_current){
    static constexpr Range duty_range = {0,1};

    auto error = targ_current - real_current;
    real_t kp_contribute = error * kp;

    real_t delta = kp_contribute;

    return duty = duty_range.clamp(duty + delta);
}