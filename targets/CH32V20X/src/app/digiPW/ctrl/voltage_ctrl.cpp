#include "voltage_ctrl.hpp"

real_t VoltageController::update(const real_t targ_volt, const real_t real_volt, const real_t real_current){
    auto error = targ_volt - real_volt;

    real_t kp_contribute = error * kp;
    real_t delta = kp_contribute;

    return targ_curr = CLAMP(targ_curr + delta, 0, curr_limit);
}