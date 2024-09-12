#include "power_ctrl.hpp"

real_t PowerController::update(const real_t targ_power, const real_t real_power, const real_t real_current){
    auto error = targ_power - real_power;
    real_t kp_contribute = error * kp;

    real_t delta = kp_contribute;

    return targ_current = CLAMP(targ_current + delta, 0, current_limit);
}